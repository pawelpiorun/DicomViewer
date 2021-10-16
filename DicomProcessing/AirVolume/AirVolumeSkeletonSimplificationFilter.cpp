#pragma once
#include "AirVolumeSkeletonSimplificationFilter.h"
#include "MinimalPathDijkstra.h"
#include "AllPathsBFS.h"

AirVolumeSkeletonSimplificationFilter::ImageType::Pointer
AirVolumeSkeletonSimplificationFilter::PrepareData(ImageType::Pointer source)
{
	auto newImage = ImageType::New();
	newImage->SetSpacing(source->GetSpacing());
	newImage->SetRegions(source->GetLargestPossibleRegion());
	newImage->Allocate();
	newImage->FillBuffer(0);

	return newImage;
}

void AirVolumeSkeletonSimplificationFilter::GenerateData()
{
	isDataReleased = false;
	skeletonImage = new DicomSeries();
	auto localInput = DicomSeriesType::New();
	localInput->Graft(const_cast<DicomSeriesType*>(this->GetInput()));
	skeletonImage->SetItkImageData(localInput);

	auto allVertices = graph->GetVertices();

	auto startVertexes = FindStarts(allVertices);
	auto endVertex = FindEnd(allVertices);

	auto minimalPathFinder = new MinimalPathDijkstra();
	auto minimalPath1 = minimalPathFinder->GetMinimalPath(
		startVertexes[0].id, endVertex.id, graph->GetAdjacencyMatrix(), graph->GetVerticesCount());
	minimalPaths.push_back(minimalPath1);
	this->UpdateProgress(0.1);

	// dump info
	std::cout << "[SimplifySkeleton] Minimal path 1: ";
	for (int i = 0; i < minimalPath1.size(); i++)
		std::cout << minimalPath1[i] << "-";
	std::cout << "done" << std::endl << "[SimplifySkeleton] Minimal cost 1: "
		<< minimalPathFinder->GetMinimalCost() << std::endl;

	auto minimalPath2 = minimalPathFinder->GetMinimalPath(
		startVertexes[1].id, endVertex.id, graph->GetAdjacencyMatrix(), graph->GetVerticesCount());
	minimalPaths.push_back(minimalPath2);
	this->UpdateProgress(0.2);

	// dump info
	std::cout << "[SimplifySkeleton] Minimal path 2: ";
	for (int i = 0; i < minimalPath2.size(); i++)
		std::cout << minimalPath2[i] << "-";
	std::cout << "done" << std::endl << "[SimplifySkeleton] Minimal cost 2: "
		<< minimalPathFinder->GetMinimalCost() << std::endl;

	//SelectPath(simplified, minimalPath1);
	//SelectPath(simplified, minimalPath2);

	CreateSimplifiedGraph();

	this->UpdateProgress(0.6);

	auto simplified = PrepareData(localInput);
	SelectSimplifiedGraph(simplified);
	this->GraftOutput(simplified);

	this->UpdateProgress(0.8);

	minimalPathsImage = new DicomSeries();
	auto img = ImageType::New();
	img->SetRegions(localInput->GetLargestPossibleRegion());
	img->SetSpacing(localInput->GetSpacing());
	img->Allocate();
	img->FillBuffer(0);

	SelectPath(img, minimalPaths[0]);
	SelectPath(img, minimalPaths[1]);
	minimalPathsImage->SetItkImageData(img);

	this->UpdateProgress(1.0);
}

void AirVolumeSkeletonSimplificationFilter::CreateSimplifiedGraph()
{
	auto sourceMatrix = graph->GetAdjacencyMatrix();
	auto sourceVertices = graph->GetVertices();

	double mean = 0;
	int count = sourceVertices.size();
	auto sum = 0;
	for (int i = 0; i < sourceVertices.size(); i++)
		sum += sourceVertices[i].index[0];
	mean = sum /= count;
	sum = 0;
	for (int i = 0; i < sourceVertices.size(); i++)
		sum += std::pow(sourceVertices[i].index[0] - mean, 2);
	auto stDev = std::sqrt(sum / (count - 1));

	std::cout << "[SimplifySkeleton] St. X dev.: " << stDev << std::endl;
	std::cout << "[SimplifySkeleton] Mean X: " << mean << std::endl;

	// vertices & indexLookup
	// k * st dev from mean
	//auto k = 1.5;

	// or (k1+k2)/2 * std dev from middle
	auto ySize = skeletonImage->GetItkImageData()
		->GetLargestPossibleRegion().GetSize()[1];
	double middle = ySize / 2;
	auto deviation = std::abs(middle - mean);
	auto k1 = ySize / (10 * deviation);
	auto k2 = ySize / (10 * stDev);
	auto k3 = (k1 + k2) / 2;

	// or just this
	auto k = 0.8 * ySize / (10 * stDev);

	std::vector<Vertex<IndexType>> simplifiedVertices;
	std::vector<IndexType> simplifiedIndexLookup;
	simplifiedIndexLookup.resize(sourceVertices.size());
	std::vector<unsigned int> toRemove;
	for (int i = 0; i < sourceVertices.size(); i++)
	{
		auto v = sourceVertices[i];
		auto in = v.index;
		//auto val = sinusesLine[0] * in[1] + sinusesLine[1];
		if (std::abs(sourceVertices[i].index[0] - middle) > k * stDev
			/*&& std::abs(sourceVertices[i].index[0] - middle) > k3 * stDev*/
			&& sourceVertices[i].index[1] < sinusesPoints[0][0]
			&& !IsOnMinimalPath(v.id)
			/*|| (v.degree == 1 && in[2] >= val - 10)*/)
		{
			//if (v.degree == 1 && in[2] >= val - 10)
			//	std::cout << "[SimplifySkeleton] Removing 1-deg vertex above sinus line: "
			//	<< v.index[0] << ", " << v.index[1] << ", " << v.index[2] << " ID: " << v.id << std::endl;

			toRemove.push_back(v.id);
			simplifiedIndexLookup[v.id] = IndexType{ -1, -1, -1 };
		}
		else
		{
			simplifiedVertices.push_back(v);
			simplifiedIndexLookup[v.id] = v.index;
		}
	}
	removedVertices = toRemove;
	std::cout << "[SimplifySkeleton] Removed vertices: " << toRemove.size() << std::endl;

	this->UpdateProgress(0.3);
	
	// adjacency matrix
	auto verticesCount = sourceVertices.size();
	auto simplifiedMatrix = new std::vector<unsigned int>*[verticesCount];
	for (int i = 0; i < verticesCount; i++)
		simplifiedMatrix[i] = new std::vector<unsigned int>[verticesCount];
	for (int i = 0; i < verticesCount; i++)
	{
		if (simplifiedIndexLookup[i][0] == -1) continue;
		for (int j = 0; j < verticesCount; j++)
		{
			if (simplifiedIndexLookup[j][0] == -1) continue;
			simplifiedMatrix[i][j] = sourceMatrix[i][j];
		}
	}

	this->UpdateProgress(0.5);

	// edges lookup
	auto simplifiedEdgesLookup = std::map<unsigned int, std::map<unsigned int, std::vector<IndexType>>>();
	auto edgesLookup = graph->GetEdgesLookup();
	for (auto it = edgesLookup.begin(); it != edgesLookup.end(); it++)
	{
		if (simplifiedIndexLookup[it->first][0] == -1) continue;

		auto subMap = it->second;
		for (auto subIt = subMap.begin(); subIt != subMap.end(); subIt++)
		{
			if (simplifiedIndexLookup[subIt->first][0] == -1) continue;
			simplifiedEdgesLookup[it->first][subIt->first] = subIt->second;
		}
	}

	simplifiedGraph = new Graph3D<IndexType>(simplifiedMatrix, simplifiedVertices, simplifiedIndexLookup, simplifiedEdgesLookup);

	// todo: update vertices degrees?
}

void AirVolumeSkeletonSimplificationFilter::SelectSimplifiedGraph(ImageType::Pointer image)
{
	auto vertices = simplifiedGraph->GetVertices();
	auto edges = simplifiedGraph->GetEdgesLookup();
	for (int i = 0; i < vertices.size(); i++)
	{
		auto id = vertices[i].id;
		auto in = vertices[i].index;
		image->SetPixel(in, _foregroundValue);

		auto neighbors = edges[id];
		if (neighbors.size() == 0)
		{
			std::cout << "[SimplifySkeleton] ERROR: Vertex has no neighbors. Vertex: " << id << "\t" << std::endl;
			/*auto adj = simplifiedGraph->GetAdjacentVertices(id);
			for (auto it = adj.begin(); it != adj.end(); it++)
				std::cout << *it << "--" << simplifiedGraph->GetEdgeIndexes(id, *it).size() << " ";
			std::cout << std::endl;*/
			//simplifiedGraph->RemoveVertex(id);
			//removedVertices.push_back(id);
			// TODO: Remove isolated vertices
		}
		else
		{
			for (int j = 0; j < neighbors.size(); j++)
			{
				auto edge = neighbors[j];
				for (int k = 0; k < edge.size(); k++)
					image->SetPixel(edge[k], _foregroundValue);
			}
		}
	}
}

void AirVolumeSkeletonSimplificationFilter::SelectPath(
	ImageType::Pointer image, std::vector<unsigned int> path)
{
	for (int i = 0; i < path.size() - 1; i++)
	{
		auto current = path[i];
		auto next = path[i + 1];
		auto in = graph->GetVertexIndex(current);
		image->SetPixel(in, _foregroundValue);
		minimalPathsVoxels.push_back(in);

		auto edgeVoxels = graph->GetEdgeIndexes(current,next);
		for (int j = 0; j < edgeVoxels.size(); j++)
		{
			image->SetPixel(edgeVoxels[j], _foregroundValue);
			minimalPathsVoxels.push_back(edgeVoxels[j]);
		}
	}
}

std::vector<Vertex<AirVolumeSkeletonSimplificationFilter::IndexType>>
AirVolumeSkeletonSimplificationFilter::FindStarts(std::vector<Vertex<IndexType>> vertices)
{
	//using ConstIteratorType = itk::ImageLinearConstIteratorWithIndex<ImageType>;
	//auto it = ConstIteratorType(skeletonImage, skeletonImage->GetLargestPossibleRegion());
	//it.GoToBegin();
	//it.SetDirection(2);
	//it.GoToBeginOfLine();

	//std::vector<IndexType> twoClosestPoints = std::vector<IndexType>();

	//while (!it.IsAtEnd() && twoClosestPoints.size() != 2)
	//{
	//	while (!it.IsAtEndOfLine())
	//	{
	//		if (it.Value() == _foregroundValue)
	//			twoClosestPoints.push_back(it.GetIndex());
	//		++it;
	//	}
	//	it.NextLine();
	//}

	std::vector<IndexType> twoClosestPoints = std::vector<IndexType>();
	twoClosestPoints.resize(2);

	auto firstMin = INT_MAX, secondMin = INT_MAX;
	for (auto it = vertices.begin(); it != vertices.end(); it++)
	{
		auto index = it->index;
		if (index[1] < firstMin)
		{
			secondMin = firstMin;
			firstMin = index[1];

			twoClosestPoints[1] = twoClosestPoints[0];
			twoClosestPoints[0] = index;
		}
		else if (index[1] < secondMin)
		{
			secondMin = index[1];
			twoClosestPoints[1] = index;
		}
	}
	auto vertexIt1 = std::find_if(vertices.begin(), vertices.end(), [twoClosestPoints](const Vertex<IndexType>& v) { return v.index == twoClosestPoints[0]; });
	auto vertexIt2 = std::find_if(vertices.begin(), vertices.end(), [twoClosestPoints](const Vertex<IndexType>& v) { return v.index == twoClosestPoints[1]; });

	std::cout << "[SimplifySkeleton] Start point 1: X = " << twoClosestPoints[0][0]
		<< " Y = " << twoClosestPoints[0][1] << " Z = " << twoClosestPoints[0][2]
		<< " Vertex: " << vertexIt1->id << std::endl;
	std::cout << "[SimplifySkeleton] Start point 2: X = " << twoClosestPoints[1][0]
		<< " Y = " << twoClosestPoints[1][1] << " Z = " << twoClosestPoints[1][2]
		<< " Vertex: " << vertexIt2->id << std::endl;

	return std::vector<Vertex<IndexType>> { *vertexIt1, *vertexIt2 };
}

Vertex<AirVolumeSkeletonSimplificationFilter::IndexType>
AirVolumeSkeletonSimplificationFilter::FindEnd(std::vector<Vertex<IndexType>> vertices)
{
	auto itkImage = skeletonImage->GetItkImageData();
	using ConstIteratorType = itk::ImageLinearConstIteratorWithIndex<ImageType>;
	auto it = ConstIteratorType(itkImage, itkImage->GetLargestPossibleRegion());
	it.GoToBegin();
	it.SetDirection(1);
	it.GoToBeginOfLine();

	IndexType bestPoint;
	bool isFound = false;
	while (!it.IsAtEnd() && !isFound)
	{
		while (!it.IsAtEndOfLine() && !isFound)
		{
			if (it.Value() == _foregroundValue)
			{
				bestPoint = it.GetIndex();
				isFound = true;
				break;
			}
			++it;
		}
		it.NextLine();
	}

	std::cout << "[SimplifySkeleton] Initial end point: X = " << bestPoint[0]
		<< " Y = " << bestPoint[1] << " Z = " << bestPoint[2] << std::endl;

	// find median XY for vertices in specified XY radius
	auto radiusY = 40;
	auto radiusZ = 40;
	std::vector<int> xarray;
	xarray.reserve(300);
	std::vector<int> yarray;
	yarray.reserve(300);
	for (int i = 0; i < vertices.size(); i++)
	{
		auto in = vertices[i].index;
		if (std::abs(in[1] - bestPoint[1]) < radiusY
			&& std::abs(in[2] - bestPoint[2]) < radiusZ)
		{
			xarray.push_back(in[0]);
			yarray.push_back(in[1]);
		}
	}
	sort(xarray.begin(), xarray.end());
	sort(yarray.begin(), yarray.end());
	auto medianX = bestPoint[0];
	auto medianY = bestPoint[1];
	auto initialZ = bestPoint[2];
	if (xarray.size() % 2 == 0) medianX = (xarray[xarray.size() / 2 - 1] + xarray[xarray.size() / 2]) / 2;
	else medianX = xarray[xarray.size() / 2];
	if (yarray.size() % 2 == 0) medianY = (yarray[yarray.size() / 2 - 1] + yarray[yarray.size() / 2]) / 2;
	else medianY = yarray[yarray.size() / 2];

	auto maxX = xarray[xarray.size() - 1], maxY = yarray[yarray.size() - 1],
		minX = xarray[0], minY = yarray[0];
	auto xSpan = (maxX - minX), ySpan = maxY - minY;
	double wage = 1.0;

	auto xWage = (double)xSpan / (xSpan + ySpan);
	auto yWage = (double)ySpan / (xSpan + ySpan);

	std::cout << "[SimplifySkeleton] Median: X = " << medianX << " Y = " << medianY
		<< " Count: " << xarray.size() << " ; " << yarray.size() << " Wages: " << xWage << " ; " << yWage << std::endl;

	double minError = std::sqrt(xWage * xWage * std::pow(bestPoint[0] - medianX, 2) + yWage * yWage * std::pow(bestPoint[1] - medianY, 2));
	for (int i = 0; i < vertices.size(); i++)
	{
		auto in = vertices[i].index;
		if (in[2] > initialZ + radiusZ) continue;
		
		auto errX = in[0] - medianX;
		auto errY = in[1] - medianY;
		auto err = std::sqrt(xWage * xWage * std::pow(errX, 2) + yWage * yWage * std::pow(errY, 2));
		std::cout << "[SimplifySkeleton] " << i << ": " << err << " PT: " << in[0] << ", " << in[1] << ", " << in[2] << std::endl;
		if (err < minError)
		{
			bestPoint = in;
			minError = err;
		}
	}

	std::cout << "[SimplifySkeleton] Final end point: X = " << bestPoint[0]
		<< " Y = " << bestPoint[1] << " Z = " << bestPoint[2] << std::endl;

	auto vertexIt = std::find_if(vertices.begin(), vertices.end(), [bestPoint](const Vertex<IndexType>& v) { return v.index == bestPoint; });
	return *vertexIt;
}

bool AirVolumeSkeletonSimplificationFilter::IsOnMinimalPath(unsigned int id)
{
	for (int i = 0; i < minimalPaths.size(); i++)
	{
		auto path = minimalPaths[i];
		for (int j = 0; j < path.size(); j++)
		{
			if (path[j] == id) return true;
		}
	}

	return false;
}

void AirVolumeSkeletonSimplificationFilter::ReleaseData()
{
	if (isDataReleased) return;

	std::cout << "Clearing simplified graph... " << std::endl;
	if (simplifiedGraph != nullptr)
	{
		delete simplifiedGraph;
		simplifiedGraph = nullptr;
	}

	if (minimalPathsImage != nullptr)
	{
		delete minimalPathsImage;
		minimalPathsImage = nullptr;
	}

	if (skeletonImage != nullptr)
	{
		delete skeletonImage;
		skeletonImage = nullptr;
	}

	std::cout << "Done." << std::endl;

	isDataReleased = true;
}

AirVolumeSkeletonSimplificationFilter::~AirVolumeSkeletonSimplificationFilter()
{
	std::cout << "SkeletonSimplification dctor... ";
	ReleaseData();
	std::cout << "done." << std::endl;
}