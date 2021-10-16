#pragma once
#include "DicomSeries.h"
#include "DicomProcessing.h"
#include "Graph3D.h"

class AirVolumeSkeletonSimplificationFilter
	: public itk::ImageToImageFilter<DicomSeriesType, DicomSeriesType>
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(AirVolumeSkeletonSimplificationFilter);

	using Self = AirVolumeSkeletonSimplificationFilter;
	using Superclass = ImageToImageFilter<DicomSeriesType, DicomSeriesType>;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(AirVolumeSkeletonSimplificationFilter);
	itkTypeMacro(AirVolumeSkeletonSimplificationFilter, ImageToImageFilter);

	using ImageType = DicomSeriesType;
	using PixelType = typename ImageType::PixelType;
	using IndexType = typename ImageType::IndexType;

	AirVolumeSkeletonSimplificationFilter() { };
	void ReleaseData();
	~AirVolumeSkeletonSimplificationFilter();

	void SetForegroundValue(ImageType::PixelType value) { _foregroundValue = value; }
	void SetSkeletonGraph(Graph3D<IndexType>* graph) { this->graph = graph; }
	void SetSinusesLine(std::vector<double> sinusesLine, std::vector<DicomImageType::IndexType> sinusesPoints)
	{ 
		this->sinusesLine = sinusesLine; 
		this->sinusesPoints = sinusesPoints;
	}
	Graph3D<IndexType>* GetSimplifiedGraph() { return this->simplifiedGraph; }
	std::vector<std::vector<unsigned int>> GetMinimalPaths() { return minimalPaths; }
	std::vector<unsigned int> GetRemovedVertices() { return removedVertices; }
	DicomSeries* GetMinimalPathsImage() { return minimalPathsImage; }
	std::vector<IndexType> GetMinimalPathsVoxels() { return this->minimalPathsVoxels; }

protected:
	void GenerateData() override;

private:
	ImageType::Pointer PrepareData(ImageType::Pointer source);
	void SelectPath(ImageType::Pointer, std::vector<unsigned int> minimalPath);
	std::vector<Vertex<IndexType>> FindStarts(std::vector<Vertex<IndexType>> vertices);
	Vertex<IndexType> FindEnd(std::vector<Vertex<IndexType>> vertices);
	void CreateSimplifiedGraph();
	void SelectSimplifiedGraph(ImageType::Pointer image);
	bool IsOnMinimalPath(unsigned int vertexId);

	std::vector<double> sinusesLine;
	std::vector<DicomImageType::IndexType> sinusesPoints;
	Graph3D<IndexType>* graph = nullptr;
	Graph3D<IndexType>* simplifiedGraph = nullptr;
	std::vector<unsigned int> removedVertices;
	std::vector<std::vector<unsigned int>> minimalPaths;
	ImageType::PixelType _foregroundValue = SHRT_MAX;
	bool isDataReleased;

	DicomSeries* minimalPathsImage = nullptr;
	DicomSeries* skeletonImage = nullptr;
	std::vector<IndexType> minimalPathsVoxels;
};