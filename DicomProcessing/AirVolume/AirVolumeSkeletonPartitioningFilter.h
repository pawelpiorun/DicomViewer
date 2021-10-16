#pragma once
#include "DicomSeries.h"
#include "DicomProcessing.h"
#include "Graph3D.h"

class AirVolumeSkeletonPartitioningFilter
	: public itk::ImageToImageFilter<DicomSeriesType, DicomSeriesType>
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(AirVolumeSkeletonPartitioningFilter);

	using Self = AirVolumeSkeletonPartitioningFilter;
	using Superclass = ImageToImageFilter<DicomSeriesType, DicomSeriesType>;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(AirVolumeSkeletonPartitioningFilter);
	itkTypeMacro(AirVolumeSkeletonPartitioningFilter, ImageToImageFilter);

	using ImageType = DicomSeriesType;
	using PixelType = typename ImageType::PixelType;
	using IndexType = typename ImageType::IndexType;

	AirVolumeSkeletonPartitioningFilter() { };
	~AirVolumeSkeletonPartitioningFilter();

	void SetForegroundValue(ImageType::PixelType value) { _foregroundValue = value; }
	void SetSkeletonGraph(Graph3D<IndexType>* graph) { this->graph = graph; }
	void SetMinimalPaths(std::vector<std::vector<unsigned int>> paths) { this->minimalPaths = paths; }
	void SetUseMinimalPaths(bool val) { this->useMinimalPaths = val; }
	void SetRemovedVertices(std::vector<unsigned int> vertices) { this->removedVertices = vertices; }
	void SetUseRemovedVertices(bool val) { this->useRemovedVertices = val; }
protected:
	void GenerateData() override;

private:
	using NeighborhoodIteratorType = itk::ConstNeighborhoodIterator<ImageType>;

	ImageType::Pointer PrepareData();
	void PartitionizeVolume();
	void CreateSkeletonPartitions();
	void CreatePartitionsOfInterestLookup();
	void CreateMinimalPathsPartitionsOfInterest();
	void CreateRemovedVerticesPartitionsOfInterest();
	void CreatePartitionLookup();
	int GetPartition(int x, int y, int z) { return partitionLookup[x][y][z]; }
	int GetPartition(const IndexType& in) { return partitionLookup[in[0]][in[1]][in[2]]; }
	void SetPartition(const IndexType& in, int id) { partitionLookup[in[0]][in[1]][in[2]] = id; }
	IndexType GetClosestSkeletonVoxel(const IndexType& index);
	IndexType GetClosestSkeletonVoxelCheckAll(const IndexType& index);
	IndexType GetClosestSkeletonVoxelCheckFromRange(const IndexType& index, int radius = 30);
	IndexType GetClosestSkeletonVoxelGrowing(const IndexType& index);
	IndexType GetClosestSkeletonVoxelCheckNeighborhood(const IndexType& index, int radius = 30);
	double Distance(const IndexType& first, const IndexType& second);

	std::vector<IndexType> skeletonVoxels;
	std::map<unsigned int, std::vector<IndexType>> skeletonPartitions;
	int*** partitionLookup = nullptr;
	unsigned int partitionSize[3];
	std::vector<bool> partsOfInterestLookup;

	ImageType::Pointer sourceVolume = nullptr;
	ImageType::Pointer partitionizedVolume = nullptr;
	std::vector<std::vector<unsigned int>> minimalPaths;
	std::vector<unsigned int> removedVertices;
	Graph3D<IndexType>* graph = nullptr;
	ImageType::PixelType _foregroundValue = SHRT_MAX;

	bool useMinimalPaths = true;
	bool useRemovedVertices = false;

	std::mutex mutex;
};