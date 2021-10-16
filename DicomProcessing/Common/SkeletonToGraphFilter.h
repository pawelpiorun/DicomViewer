#pragma once
#include "DicomSeries.h"
#include "DicomProcessing.h"
#include "Graph3D.h"

class SkeletonToGraph
{
public:
	SkeletonToGraph() { };
	~SkeletonToGraph();

	using ImageType = DicomSeriesType;
	using IndexType = DicomSeriesType::IndexType;

	void SetInput(DicomSeriesType::Pointer skeletonImage)
	{
		this->skeletonImage = DicomProcessing::Copy<ImageType>(skeletonImage);
	}
	void SetForegroundValue(ImageType::PixelType value) { _foregroundValue = value; }
	Graph3D<IndexType>* GetGraph();

private:
	using NeighborhoodIteratorType = itk::ConstNeighborhoodIterator<ImageType>;

	int*** CreateVertices();
	std::vector<unsigned int>** CreateAdjacencyMatrix(int*** vertexLookup);
	bool IsVertex(NeighborhoodIteratorType nit, IndexType index, int& vertexDegree);
	void ComputeVertex(Vertex<IndexType> v, std::vector<unsigned int>** adjMatrix, NeighborhoodIteratorType nit, int*** vertexLookup);
	std::map<unsigned int, std::vector<unsigned int>> GetEdgesCosts(Vertex<IndexType> v, std::vector<unsigned int>** adjMatrix, int verticesCount);
	int*** CreateVertexLookup();

	std::vector<Vertex<IndexType>> allVertices;
	std::vector<IndexType> indexLookup;
	std::map<unsigned int, std::map<unsigned int, std::vector<IndexType>>> edgesLookup;

	DicomSeriesType::Pointer skeletonImage;
	ImageType::PixelType _foregroundValue = SHRT_MAX;
};