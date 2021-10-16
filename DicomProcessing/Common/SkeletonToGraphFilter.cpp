#include "SkeletonToGraphFilter.h"

Graph3D<SkeletonToGraph::IndexType>* SkeletonToGraph::GetGraph()
{
	auto vertexLookup = CreateVertices();

	std::vector<unsigned int>** adjacencyMatrix = CreateAdjacencyMatrix(vertexLookup);

	auto size = skeletonImage->GetLargestPossibleRegion().GetSize();
	for (int x = 0; x < size[0]; x++)
	{
		for (int y = 0; y < size[1]; y++)
			delete[] vertexLookup[x][y];
		delete[] vertexLookup[x];
	}
	delete[] vertexLookup;
	vertexLookup = nullptr;

	auto graph = new Graph3D<IndexType>(adjacencyMatrix, allVertices, indexLookup, edgesLookup);
	return graph;
}

std::vector<unsigned int>** SkeletonToGraph::CreateAdjacencyMatrix(int*** vertexLookup)
{
	auto verticesCount = allVertices.size();
	auto adjacencyMatrix = new std::vector<unsigned int>*[verticesCount];
	for (int i = 0; i < verticesCount; i++)
		adjacencyMatrix[i] = new std::vector<unsigned int>[verticesCount];

	for (int i = 0; i < verticesCount; i++)
		for (int j = 0; j < verticesCount; j++)
			adjacencyMatrix[i][j] = std::vector<unsigned int>();

	NeighborhoodIteratorType::RadiusType radius;
	for (unsigned int i = 0; i < ImageType::ImageDimension; ++i) radius[i] = 1;
	auto nit = NeighborhoodIteratorType(radius, skeletonImage, skeletonImage->GetLargestPossibleRegion());

	for (int i = 0; i < allVertices.size(); i++)
	{
		auto vertex = allVertices[i];
		ComputeVertex(vertex, adjacencyMatrix, nit, vertexLookup);
	}
	return adjacencyMatrix;
}

void SkeletonToGraph::ComputeVertex(
	Vertex<IndexType> v, std::vector<unsigned int>** adjMatrix,
	NeighborhoodIteratorType nit, int*** vertexLookup)
{
	auto vId = v.id;
	auto vIn = v.index;
	nit.SetLocation(vIn);
	if (nit.GetCenterPixel() == 1) return; // already done

	// set as done
	skeletonImage->SetPixel(vIn, 1);

	for (int i = 0; i < nit.Size(); i++)
	{
		if (nit.GetPixel(i) == _foregroundValue)
		{
			auto in = nit.GetIndex(i);
			auto id = vertexLookup[in[0]][in[1]][in[2]];
			if (id == -1)
			{
				// compute edge
				auto edgeCount = 0;
				auto subId = -1;
				std::vector<IndexType> edgeVoxels;
				while (subId == -1)
				{
					// current voxel
					nit.SetLocation(in);
					subId = vertexLookup[in[0]][in[1]][in[2]];

					// found vertex end
					if (subId > -1) break;

					// mark as edge voxel
					edgeVoxels.push_back(in);
					skeletonImage->SetPixel(in, 1);
					edgeCount++;

					// find next edge voxel
					auto current = in;
					bool nextFound = false;
					for (int j = 0; j < nit.Size(); j++)
					{
						auto temp = nit.GetIndex(j);
						if (nit.GetPixel(j) == _foregroundValue)
						{
							in = temp;
							nextFound = true;
							break;
						}
					}

					if (!nextFound)
					{
						bool isLoop = false;
						for (int j = 0; j < nit.Size(); j++)
						{
							auto temp = nit.GetIndex(j);
							if (temp == indexLookup[vId])
							{
								std::cout << "[SimplifySkeleton] Loop encountered. ID = " << vId << std::endl;
								isLoop = true;
							}
						}

						if (isLoop)
						{
							std::cout << "[SimplifySkeleton] ERROR: No vertex found on the end of the edge. Setting edge as a loop." << std::endl;
							subId = vId;
							break;
						}

						std::cout << "[SimplifySkeleton] ERROR: No vertex found on the end of the edge. Removing edge." << std::endl;
						for (int k = 0; k < edgeVoxels.size(); k++)
							skeletonImage->SetPixel(edgeVoxels[k], 0);

						// mark last edge voxel as a new vertex
						//edgeVoxels.pop_back();
						//edgeCount--;
						//auto verticesCount = allVertices.size();
						//std::cout << "ID = " << verticesCount << " Existing: "
						//	<< (indexLookup.size() > verticesCount ? "true! ERROR" : "false. OK") << std::endl;
						//auto newVertex = Vertex<IndexType>(1, current, verticesCount);
						//allVertices.push_back(newVertex);
						//vertexLookup[current[0]][current[1]][current[2]] = verticesCount;
						//indexLookup.push_back(current);
						//subId = verticesCount;

						break;
					}
				}

				if (subId != -1)
				{
					// store edge indexes
					edgesLookup[vId][subId] = edgeVoxels;
					std::reverse(edgeVoxels.begin(), edgeVoxels.end());
					edgesLookup[subId][vId] = edgeVoxels;

					// store costs
					adjMatrix[vId][subId].push_back(edgeCount);
					adjMatrix[subId][vId].push_back(edgeCount);
				}

				// set center pixel back to the vertex
				nit.SetLocation(vIn);
			}
			else
			{
				// adjacent vertices
				adjMatrix[vId][id].push_back(0);
				adjMatrix[id][vId].push_back(0);
				edgesLookup[vId][id] = std::vector<IndexType>();
				edgesLookup[id][vId] = std::vector<IndexType>();
			}
		}
	}
}

int*** SkeletonToGraph::CreateVertices()
{
	auto vertexLookup = CreateVertexLookup();

	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto it = ConstIteratorType(skeletonImage, skeletonImage->GetLargestPossibleRegion());
	it.GoToBegin();

	NeighborhoodIteratorType::RadiusType radius;
	for (unsigned int i = 0; i < ImageType::ImageDimension; ++i) radius[i] = 1;
	auto nit = NeighborhoodIteratorType(radius, skeletonImage, skeletonImage->GetLargestPossibleRegion());

	allVertices = std::vector<Vertex<IndexType>>();
	auto pixelCount = 0;
	auto idCounter = 0;
	while (!it.IsAtEnd())
	{
		if (it.Value() == _foregroundValue)
		{
			pixelCount++;
			int vertexDegree = 0;
			auto in = it.GetIndex();
			auto isVertex = IsVertex(nit, in, vertexDegree);
			if (isVertex)
			{
				auto vertex = Vertex<IndexType>(vertexDegree, in, idCounter);
				allVertices.push_back(vertex);
				vertexLookup[in[0]][in[1]][in[2]] = idCounter;
				indexLookup.push_back(in);
				idCounter++;
			}
		}
		++it;
	}

	std::cout << "[SkeletonToGraph] Pixel count: = " << pixelCount << std::endl;
	std::cout << "[SkeletonToGraph] All vertices count: = " << allVertices.size() << std::endl;

	return vertexLookup;
}

bool SkeletonToGraph::IsVertex(NeighborhoodIteratorType nit, IndexType index, int& vertexDegree)
{
	nit.SetLocation(index);
	auto count = 0;
	for (unsigned int i = 0; i < nit.Size(); i++)
	{
		if (nit.GetPixel(i) != _foregroundValue) continue;

		auto in = nit.GetIndex(i);
		if (in == index)
			continue;

		count++;
	}
	vertexDegree = count;
	return count > 0 && count != 2;
}

std::map<unsigned int, std::vector<unsigned int>>
SkeletonToGraph::GetEdgesCosts(
	Vertex<IndexType> v, std::vector<unsigned int>** adjMatrix, int verticesCount)
{
	auto id = v.id;
	auto edges = std::map<unsigned int, std::vector<unsigned int>>();
	for (int i = 0; i < verticesCount; i++)
	{
		auto elem = adjMatrix[id][i];
		if (adjMatrix[id][i].size() > 0)
		{
			edges[i] = adjMatrix[id][i];
		}
	}

	return edges;
}

int*** SkeletonToGraph::CreateVertexLookup()
{
	// 3d array with isVertex flag
	auto size = skeletonImage->GetLargestPossibleRegion().GetSize();
	auto vertexLookup = new int**[size[0]];
	for (int x = 0; x < size[0]; x++)
	{
		vertexLookup[x] = new int*[size[1]];
		for (int y = 0; y < size[1]; y++)
		{
			vertexLookup[x][y] = new int[size[2]];
			for (int z = 0; z < size[2]; z++)
				vertexLookup[x][y][z] = -1;
		}
	}

	return vertexLookup;
}

SkeletonToGraph::~SkeletonToGraph()
{
	std::cout << "SkeletonToGraph dctor... done." << std::endl;
}