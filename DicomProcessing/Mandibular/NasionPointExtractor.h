#pragma once
#include "DicomProcessing.h"
#include "DicomImage.h"
#include "DicomSeries.h"
#include "PointsToLineLeastSquaresAlgorithm.h"

class NasionPointExtractor : public itk::ProcessObject
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(NasionPointExtractor);

	using Self = NasionPointExtractor;
	using Superclass = Object;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(NasionPointExtractor);

	using ImageType = DicomSeriesType;
	using PointerType = typename ImageType::Pointer;
	using IndexType = typename ImageType::IndexType;
	using PixelType = typename ImageType::PixelType;

	NasionPointExtractor() { };
	~NasionPointExtractor();

	void SetImage(ImageType::Pointer image);
	void SetForegroundValue(ImageType::PixelType value) { this->foregroundValue = value; }
	void SetZThreshold(int z) { this->zThreshold = z; }
	void Compute();
	IndexType GetNasionPoint() { return nasionPoint; }
	DicomImage* GetUpperSkullImage() { return this->upperSkullProjection; }

private:
	void CreateYZProjection();
	void CalculateNasion();
	void SelectNasionLevel();
	DicomImageType::IndexType GetMinimalYProjectionPoint();
	IndexType GetNasionPointFromProjected(DicomImageType::IndexType projected);

	int zThreshold = 0;
	ImageType::PixelType foregroundValue = SHRT_MAX;
	DicomSeries* image = nullptr;
	DicomImage* upperSkullProjection = nullptr;

	ImageType::IndexType nasionPoint;
};