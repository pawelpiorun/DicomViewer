#ifndef MINIMALCROSSSECTIONFINDER_HEADER
#define MINIMALCROSSSECTIONFINDER_HEADER

#pragma once
#include "itkCommon.h"
#include "common.h"
#include "Graph3D.h"
#include "ITK-5.1/itkNeighborhoodIterator.h"
#include "RgbImage.h"

class MinimalCrossSectionFinder
{
public:
	MinimalCrossSectionFinder() { };
	~MinimalCrossSectionFinder();

	void SetInput(DicomSeriesType::Pointer image) { this->image = image; imageSize = image->GetLargestPossibleRegion().GetSize(); }
	void SetMinimalPaths(std::vector<std::vector<unsigned int>> paths) { this->minimalPaths = paths; }
	void SetMinimalPathsImage(DicomSeriesType::Pointer image) { this->minimalPathsImage = image; }
	double GetMinimalCrossSection() { return calculatedSection; }
	double GetMinimalCrossSection2() { return calculatedSection2; }
	double GetMinimalCrossSection3() { return calculatedSection3; }
	double GetMinimalCrossSection4() { return calculatedSection4; }
	RgbImage* GetMinimalCrossSectionFrontImage();
	RgbImage* GetMinimalCrossSectionSideImage();
	void Compute();

	std::vector<double> GetMinimalCrossSections() { return this->minimalSections; }
	std::vector<double> GetMinimalCrossSections2() { return this->minimalSections2; }
	std::vector<double> GetMinimalCrossSections3() { return this->minimalSections3; }
	std::vector<double> GetMinimalCrossSections4() { return this->minimalSections4; }
	std::vector<DicomSeriesType::IndexType> GetMinimalCrossSectionsCoords() { return this->minimalCrossSectionsCoords; }
	std::vector<std::vector<double>> GetMinimalCrossSectionsNormals() { return this->minimalCrossSectionsNormals; }

	DicomSeriesType::IndexType GetSplitPlanePoint() { return splitPlanePoint; }
	std::vector<double> GetSplitPlaneNormal() { return splitPlaneNormal; }


private:
	using ImageType = DicomSeriesType;
	using IndexType = ImageType::IndexType;
	using NeighborhoodIteratorType = itk::NeighborhoodIterator<ImageType>;

	RgbImage* GetCrossSectionImage(bool front = false);

	void ComputeUsingLocalGraph();

	// Line fitted to all points + voxel count
	void CalculateMinimalCrossSection(std::vector<IndexType> points);

	// Locally fitted lines + voxel counts
	void CalculateMinimalCrossSection2(std::vector<IndexType> points);

	// Line fitted to all points, but with Y normal = 0 + voxel counts
	void CalculateMinimalCrossSection3(std::vector<IndexType> points);

	// Locally fitted lines, but with Y normal = 0 + voxel counts
	void CalculateMinimalCrossSection4(std::vector<IndexType> points);

	// Locally fitted lines + area based on ellipse diagonals
	void CalculateMinimalCrossSection5(std::vector<IndexType> points);

	// Locally fitted lines + area based on each voxel double integral from plane <3
	void CalculateMinimalCrossSection6(std::vector<IndexType> points);

	// based on normal and voxel count laying on planes
	std::vector<double> CalculateCrossSections(std::vector<IndexType> anchorPoints, std::vector<double> normal);
	std::vector<double> CalculateCrossSections2(
		std::vector<IndexType> anchorPoints, std::vector<std::vector<double>> normals);

	// based on ellipse diagonals
	std::vector<double> CalculateCrossSections3(
		std::vector<IndexType> anchorPoints, std::vector<std::vector<double>> normals);


	// sum of double integral from plane for each voxel
	std::vector<double> CalculateCrossSections4(
		std::vector<IndexType> anchorPoints, std::vector<std::vector<double>> normals);

	std::vector<IndexType> GetBoundaryVoxels(std::vector<IndexType> voxels);
	double GetEllipseArea(std::vector<IndexType> points);

	std::vector<std::vector<double>> CalculateLocalNormals(std::vector<IndexType> anchorPoints);
	bool IsOnPlane(IndexType point, IndexType anchorPoint, std::vector<double> normal);
	bool IsOnPlane2(IndexType point, IndexType anchorPoint, std::vector<double> normal);
	bool IsOnPlane3(IndexType point, IndexType anchorPoint, std::vector<double> normal);
	double PointToPlaceDist(IndexType point, IndexType anchorPoint, std::vector<double> normal);
	double MinimalCrossSectionFinder::PointToPointDistXY(double* a, double* b);
	std::vector<double*> SortPoints(std::vector<double*> points);
	double PolygonArea(std::vector<double*> sortedPoints);
	double VoxelCrossSectionRealArea(IndexType point, IndexType anchorPoint, std::vector<double> normal, itk::Vector<double, 3U> spacing);

	bool IsOnLine(IndexType point, IndexType anchorPoint, std::vector<double> normal, double* err = nullptr);
	Graph3D<IndexType>* CreateMinimalPathsGraph();
	std::vector<IndexType> FilterCrossSection(std::vector<IndexType> voxels, IndexType anchorPoint, int index);
	std::vector<IndexType> FilterCrossSection2(std::vector<IndexType> voxels, IndexType anchorPoint, int index);

	// consumes
	std::vector<std::vector<unsigned int>> minimalPaths;
	DicomSeriesType::Pointer image = nullptr;
	DicomSeriesType::SizeType imageSize;
	DicomSeriesType::Pointer minimalPathsImage = nullptr;

	// produces
	double calculatedSection = 0;
	IndexType calculatedSectionPoint;
	std::vector<double> calculatedSectionNormal;
	double calculatedSection2 = 0;
	IndexType calculatedSectionPoint2;
	std::vector<double> calculatedSectionNormal2;
	double calculatedSection3 = 0;
	IndexType calculatedSectionPoint3;
	std::vector<double> calculatedSectionNormal3;

	double calculatedSection4 = 0;
	IndexType calculatedSectionPoint4;
	std::vector<double> calculatedSectionNormal4;

	std::vector<double> fittedLine;

	std::vector<double> minimalSections;
	std::vector<double> minimalSections2;
	std::vector<double> minimalSections3;
	std::vector<double> minimalSections4;
	std::vector<DicomSeriesType::IndexType> minimalCrossSectionsCoords;
	std::vector<std::vector<double>> minimalCrossSectionsNormals;

	IndexType splitPlanePoint;
	std::vector<double> splitPlaneNormal;
	//RgbImage* frontImage;
	//RgbImage* sideImage;
};

#endif