#include "AirVolumeSkeletonPartitioningFilter.h"
#include <thread>
#include <algorithm>
#include <functional>
#include <windows.h>
#include <ppl.h>

AirVolumeSkeletonPartitioningFilter::ImageType::Pointer
AirVolumeSkeletonPartitioningFilter::PrepareData()
{
	auto newImage = ImageType::New();
	newImage->SetSpacing(sourceVolume->GetSpacing());
	newImage->SetRegions(sourceVolume->GetLargestPossibleRegion());
	newImage->Allocate();
	newImage->FillBuffer(0);

	return newImage;
}

void AirVolumeSkeletonPartitioningFilter::GenerateData()
{
	auto input = DicomSeriesType::New();
	input->Graft(const_cast<DicomSeriesType*>(this->GetInput()));
	sourceVolume = input;
	if (sourceVolume == nullptr)
		throw "[SkeletonPartitioning] Source volume is null.";

	PartitionizeVolume();
	if (partitionizedVolume != nullptr)
	{
		this->GraftOutput(partitionizedVolume);
		std::cout << "[SkeletonPartitioning] Partitioning done. " << std::endl;
	}
	else
		throw "[SkeletonPartitioning] Partitionized volume is null.";
}

void AirVolumeSkeletonPartitioningFilter::PartitionizeVolume()
{
	partitionizedVolume = PrepareData();
	CreateSkeletonPartitions();

	CreatePartitionsOfInterestLookup();
	this->UpdateProgress(0.1);

	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	//using IteratorType = itk::ImageRegionIterator<ImageType>;
	//auto it = IteratorType(partitionizedVolume, partitionizedVolume->GetLargestPossibleRegion());
	auto sourceIt = ConstIteratorType(sourceVolume, sourceVolume->GetLargestPossibleRegion());

	//NeighborhoodIteratorType::RadiusType radius;
	//for (int i = 0; i < ImageType::ImageDimension; i++) radius[i] = 0;
	//auto nit = NeighborhoodIteratorType(radius, sourceVolume, sourceVolume->GetLargestPossibleRegion());
	//it.GoToBegin();
	sourceIt.GoToBegin();
	//nit.GoToBegin();

	std::vector<IndexType> toCheck;
	while (/*!it.IsAtEnd() &&*/ !sourceIt.IsAtEnd())
	{
		if (sourceIt.Value() == _foregroundValue)
		{
			toCheck.push_back(sourceIt.GetIndex());
			//auto closest = GetClosestSkeletonVoxel(in);
			//auto partition = GetPartition(closest);
			//if (partition == -1)
			//	it.Set(0);
			//else if (partsOfInterestLookup[partition])
			//	it.Set(_foregroundValue);
			//else
			//	it.Set(0);
		}
		//++it;
		++sourceIt;
	}

	// TODO: secure multithreading
	float progress = 0.1;
	float step = 0.9 / toCheck.size();
	concurrency::parallel_for_each(toCheck.begin(), toCheck.end(), [&](IndexType in)
		{
			auto closest = GetClosestSkeletonVoxel(in);
			auto partition = GetPartition(closest);

			mutex.lock();
			if (partition != -1)
			{
				if (partsOfInterestLookup[partition])
					partitionizedVolume->SetPixel(in, _foregroundValue);
				else
					partitionizedVolume->SetPixel(in, 0);
			}

			progress += step;
			this->UpdateProgress(progress);
			mutex.unlock();
		});
}

void AirVolumeSkeletonPartitioningFilter::CreatePartitionsOfInterestLookup()
{
	partsOfInterestLookup.resize(graph->GetVerticesCount());
	if (useMinimalPaths)
		CreateMinimalPathsPartitionsOfInterest();
	else if (useRemovedVertices)
		CreateRemovedVerticesPartitionsOfInterest();
	else
		std::fill(partsOfInterestLookup.begin(), partsOfInterestLookup.end(), true);

	std::cout << "[SkeletonPartitioning] Lookup size: " << partsOfInterestLookup.size() << std::endl;
}

void AirVolumeSkeletonPartitioningFilter::CreateMinimalPathsPartitionsOfInterest()
{
	std::fill(partsOfInterestLookup.begin(), partsOfInterestLookup.end(), false);
	for (int i = 0; i < minimalPaths.size(); i++)
	{
		auto path = minimalPaths[i];
		for (int j = 0; j < path.size() - 1; j++)
		{
			auto id = path[j];
			auto next = path[j + 1];
			auto in = graph->GetVertexIndex(id);
			auto nextIn = graph->GetVertexIndex(next);
			auto partition = GetPartition(in);
			auto nextPartition = GetPartition(nextIn);

			// check
			if (partition == -1)
				std::cout << "[SkeletonPartitioning] Wrong vertex partition: "
				<< in[0] << ", " << in[1] << ", " << in[2] << " Vertex: " << id << std::endl;
			if (nextPartition == -1)
				std::cout << "[SkeletonPartitioning] Wrong next vertex partition: "
				<< nextIn[0] << ", " << nextIn[1] << ", " << nextIn[2] << " Vertex: " << next << std::endl;

			partsOfInterestLookup[partition] = true;
			partsOfInterestLookup[nextPartition] = true;

			auto edge = graph->GetEdgeIndexes(id, next);
			for (int k = 0; k < edge.size(); k++)
			{
				auto edgeIn = edge[k];
				auto edgePartition = GetPartition(edgeIn);

				if (edgePartition == -1)
				{
					std::cout << "[SkeletonPartitioning] Wrong edge partition: "
						<< edgeIn[0] << ", " << edgeIn[1] << ", " << edgeIn[2]
						<< " Vertices: " << id << " - " << next
						<< " Partitions: " << partition << " - " << nextPartition << std::endl;

					if (partition == nextPartition)
					{
						SetPartition(edgeIn, partition);
						edgePartition = partition;
					}
				}
				partsOfInterestLookup[edgePartition] = true;
			}
		}
	}

	// set neighbors parts as interest as well ?
	//for (int i = 0; i < minimalPaths.size(); i++)
	//{
	//	auto path = minimalPaths[i];
	//	for (int j = 0; j < path.size(); j++)
	//	{
	//		auto id = path[j];
	//		auto rootPart = GetPartition(graph->GetVertexIndex(id));
	//		auto neighbors = graph->GetAdjacentVertices(id);
	//		for (int k = 0; k < neighbors.size(); k++)
	//		{
	//			auto in = graph->GetVertexIndex(neighbors[k]);
	//			auto part = GetPartition(in);
	//			if (part == rootPart) continue;

	//			partsOfInterestLookup[part] = true;
	//		}

	//		for (int k = 0; k < neighbors.size(); k++)
	//		{
	//			auto edge = graph->GetEdgeIndexes(id, neighbors[k]);
	//			for (int e = 0; e < edge.size(); e++)
	//			{
	//				auto part = GetPartition(edge[e]);
	//				if (part == rootPart) continue;
	//				partsOfInterestLookup[part] = true;
	//			}
	//		}
	//	}
	//}

}

void AirVolumeSkeletonPartitioningFilter::CreateRemovedVerticesPartitionsOfInterest()
{
	// set everything interesting except removed vertices and their all edges
	std::fill(partsOfInterestLookup.begin(), partsOfInterestLookup.end(), true);
	for (auto it = removedVertices.begin(); it != removedVertices.end(); it++)
	{
		auto id = *it;
		auto in = graph->GetVertexIndex(id);
		auto partition = GetPartition(in);

		// check
		if (partition == -1)
			std::cout << "[SkeletonPartitioning] Wrong vertex partition: "
			<< in[0] << ", " << in[1] << ", " << in[2] << " Vertex: " << id << std::endl;

		partsOfInterestLookup[partition] = false;

		auto edgesMap = graph->GetEdgesLookup()[id];
		for (auto neighborIt = edgesMap.begin(); neighborIt != edgesMap.end(); neighborIt++)
		{
			auto neighborId = neighborIt->first;
			auto neighborPartition = GetPartition(graph->GetVertexIndex(neighborId));
			auto edgeVoxels = neighborIt->second;
			for (int i = 0; i < edgeVoxels.size(); i++)
			{
				auto edgeIn = edgeVoxels[i];
				auto edgePartition = GetPartition(edgeIn);
				if (edgePartition == -1)
				{
					std::cout << "[SkeletonPartitioning] Wrong edge partition: "
						<< edgeIn[0] << ", " << edgeIn[1] << ", " << edgeIn[2]
						<< " Vertices: " << id << " - " << neighborId
						<< " Partitions: " << partition << " - " << neighborPartition << std::endl;

					if (partition == neighborPartition)
					{
						SetPartition(edgeIn, partition);
						edgePartition = partition;
					}
				}
				partsOfInterestLookup[edgePartition] = false;
			}
		}
	}
}

AirVolumeSkeletonPartitioningFilter::IndexType
AirVolumeSkeletonPartitioningFilter::GetClosestSkeletonVoxel(const IndexType& in)
{
	auto partition = GetPartition(in);
	if (partition >= 0)
		return in;

	// too slow
	//return GetClosestSkeletonVoxelCheckAll(in);

	// twice faster as before, but still too slow
	//return GetClosestSkeletonVoxelCheckFromRange(in);

	// much faster!
	return GetClosestSkeletonVoxelCheckNeighborhood(in);

	// TODO?
	//return GetClosestSkeletonVoxelVoxelGrowing(in);
}

AirVolumeSkeletonPartitioningFilter::IndexType
AirVolumeSkeletonPartitioningFilter::GetClosestSkeletonVoxelCheckAll(const IndexType& in)
{
	IndexType closest;
	auto minDist = DBL_MAX;
	for (auto it = skeletonVoxels.begin(); it != skeletonVoxels.end(); it++)
	{
		auto curr = *it;
		auto dist = Distance(curr, in);
		if (dist < minDist)
		{
			minDist = dist;
			closest = curr;
		}
	}
	return closest;
}

AirVolumeSkeletonPartitioningFilter::IndexType
AirVolumeSkeletonPartitioningFilter::GetClosestSkeletonVoxelCheckFromRange(
	const IndexType& in, int radius)
{
	IndexType closest;
	auto minDist = DBL_MAX;
	for (auto it = skeletonVoxels.begin(); it != skeletonVoxels.end(); it++)
	{
		auto curr = *it;
		if (std::abs(curr[0] - in[0]) < radius
			|| std::abs(curr[1] - in[1]) < radius
			|| std::abs(curr[2] - in[2]) < radius)
		{
			auto dist = Distance(curr, in);
			if (dist < minDist)
			{
				minDist = dist;
				closest = curr;
			}
		}
	}
	return closest;
}

AirVolumeSkeletonPartitioningFilter::IndexType
AirVolumeSkeletonPartitioningFilter::GetClosestSkeletonVoxelCheckNeighborhood(
	const IndexType& in, int radius)
{
	IndexType closest;
	auto minDist = DBL_MAX;
	for (auto it = skeletonVoxels.begin(); it != skeletonVoxels.end(); it++)
	{
		auto curr = *it;
		if (std::abs(curr[0] - in[0]) < radius
			&& std::abs(curr[1] - in[1]) < radius
			&& std::abs(curr[2] - in[2]) < radius)
		{
			auto dist = Distance(curr, in);
			if (dist < minDist)
			{
				minDist = dist;
				closest = curr;
			}
		}
	}
	return closest;
}

AirVolumeSkeletonPartitioningFilter::IndexType
AirVolumeSkeletonPartitioningFilter::GetClosestSkeletonVoxelGrowing(const IndexType& in)
{
	// TODO 
	IndexType closest;



	return closest;
}

double AirVolumeSkeletonPartitioningFilter::Distance(
	const IndexType& first, const IndexType& second)
{
	return std::sqrt(
		std::pow(first[0] - second[0], 2)
		+ std::pow(first[1] - second[1], 2)
		+ std::pow(first[2] - second[2], 2));
}

void AirVolumeSkeletonPartitioningFilter::CreateSkeletonPartitions()
{
	CreatePartitionLookup();

	auto partitionId = 0;
	std::vector<std::vector<IndexType>> minimalPathEdges;
	for (int i = 0; i < minimalPaths.size(); i++)
	{
		auto path = minimalPaths[i];
		for (int j = 0; j < path.size() - 1; j++)
		{
			auto in = graph->GetVertexIndex(path[j]);
			auto vertexPartition = partitionId;
			auto storedPartition = GetPartition(in);
			if (storedPartition != -1)
				vertexPartition = storedPartition;

			std::vector<IndexType> vertexCluster;
			vertexCluster.push_back(in);
			SetPartition(in, vertexPartition);
			skeletonVoxels.push_back(in);

			auto adjacentVertices = graph->GetAdjacentVertices(path[j]);
			for (int n = 0; n < adjacentVertices.size(); n++)
			{
				auto edgeVoxels = graph->GetEdgeIndexes(path[j], adjacentVertices[n]);
				if (edgeVoxels.size() == 0)
				{
					// add neighbor vertex to the group
					auto adjIn = graph->GetVertexIndex(adjacentVertices[n]);
					SetPartition(adjIn, vertexPartition);
					skeletonVoxels.push_back(adjIn);
					vertexCluster.push_back(adjIn);
				}
				else
				{
					minimalPathEdges.push_back(edgeVoxels);
					// add 1/2 of all edges to the cluster
					for (int k = 0; k < edgeVoxels.size() / 2; k++)
					{
						auto edgIn = edgeVoxels[k];
						SetPartition(edgIn, vertexPartition);
						skeletonVoxels.push_back(edgIn);
						vertexCluster.push_back(edgIn);
					}
				}
			}

			if (storedPartition != -1)
			{
				for (int v = 0; v < vertexCluster.size(); v++)
					skeletonPartitions[vertexPartition].push_back(vertexCluster[v]); // may have duplicates
			}
			else
			{
				skeletonPartitions[partitionId] = vertexCluster;
				partitionId++;
			}

		}
	}

	auto vertices = graph->GetVertices();
	std::vector<std::vector<IndexType>> edges;
	for (int i = 0; i < vertices.size(); i++)
	{
		auto vertex = vertices[i];
		auto in = vertex.index;
		auto vertexPartition = partitionId;
		auto storedPartition = GetPartition(in);
		if (storedPartition != -1)
			vertexPartition = storedPartition;

		std::vector<IndexType> vertexCluster;
		vertexCluster.push_back(in);
		SetPartition(in, vertexPartition);
		skeletonVoxels.push_back(in);

		// todo remove
		//partitionizedVolume->SetPixel(in, SHRT_MAX);

		auto adjacentVertices = graph->GetAdjacentVertices(vertex.id);
		for (int j = 0; j < adjacentVertices.size(); j++)
		{
			auto edgeVoxels = graph->GetEdgeIndexes(vertex.id, adjacentVertices[j]);
			if (edgeVoxels.size() == 0)
			{
				// add neighbor vertex to the group
				auto adjIn = graph->GetVertexIndex(adjacentVertices[j]);
				SetPartition(adjIn, vertexPartition);
				skeletonVoxels.push_back(adjIn);
				vertexCluster.push_back(adjIn);

				// todo remove
				//partitionizedVolume->SetPixel(adjIn, SHRT_MAX);
			}
			else
			{
				edges.push_back(edgeVoxels);
				// add 1/3 of all edges to the cluster
				for (int k = 0; k < edgeVoxels.size() / 3; k++)
				{
					auto edgIn = edgeVoxels[k];
					SetPartition(edgIn, vertexPartition);
					skeletonVoxels.push_back(edgIn);
					vertexCluster.push_back(edgIn);


					// todo remove
					//partitionizedVolume->SetPixel(edgIn, SHRT_MAX);
				}
			}
		}

		if (storedPartition != -1)
		{
			for (int v = 0; v < vertexCluster.size(); v++)
				skeletonPartitions[vertexPartition].push_back(vertexCluster[v]);
		}
		else
		{
			skeletonPartitions[partitionId] = vertexCluster;
			partitionId++;
		}
	}

	for (int i = 0; i < minimalPathEdges.size(); i++)
	{
		bool hasElements = false;
		auto edge = minimalPathEdges[i];
		for (int j = 0; j < edge.size(); j++)
		{
			auto in = edge[j];
			if (GetPartition(in) != -1) continue;

			SetPartition(in, partitionId);
			hasElements = true;

			// todo remove
			//partitionizedVolume->SetPixel(in, SHRT_MAX);
		}

		if (hasElements)
		{
			skeletonPartitions[partitionId] = edge;
			partitionId++;
		}
	}

	for (int i = 0; i < edges.size(); i++)
	{
		bool hasElements = false;
		auto edge = edges[i];
		for (int j = 0; j < edge.size(); j++)
		{
			auto in = edge[j];
			if (GetPartition(in) != -1) continue;

			SetPartition(in, partitionId);
			hasElements = true;

			// todo remove
			//partitionizedVolume->SetPixel(in, SHRT_MAX);
		}

		if (hasElements)
		{
			skeletonPartitions[partitionId] = edge;
			partitionId++;
		}
	}

	std::cout << "[SkeletonPartitioning] Partitions count: " << partitionId - 2 << std::endl;
}

void AirVolumeSkeletonPartitioningFilter::CreatePartitionLookup()
{
	auto sourceSize = sourceVolume->GetLargestPossibleRegion().GetSize();
	partitionSize[0] = sourceSize[0];
	partitionSize[1] = sourceSize[1];
	partitionSize[2] = sourceSize[2];
	partitionLookup = new int**[partitionSize[0]];
	for (int x = 0; x < partitionSize[0]; x++)
	{
		partitionLookup[x] = new int*[partitionSize[1]];
		for (int y = 0; y < partitionSize[1]; y++)
		{
			partitionLookup[x][y] = new int[partitionSize[2]];
			for (int z = 0; z < partitionSize[2]; z++)
				partitionLookup[x][y][z] = -1;
		}
	}
}

AirVolumeSkeletonPartitioningFilter::~AirVolumeSkeletonPartitioningFilter()
{
	std::cout << "SkeletonPartitioning dctor... ";
	graph = nullptr;
	if (partitionLookup != nullptr)
	{
		for (int x = 0; x < partitionSize[0]; x++)
		{
			for (int y = 0; y < partitionSize[1]; y++)
				delete[] partitionLookup[x][y];
			delete[] partitionLookup[x];
		}
		delete[] partitionLookup;
		partitionLookup = nullptr;
	}
	std::cout << "done." << std::endl;
}