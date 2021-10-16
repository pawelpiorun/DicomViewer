#ifndef ZYGOMATICARCH_HEADER
#define ZYGOMATICARCH_HEADER

#pragma once
#include "DicomImage.h"
#include "DicomSeries.h"
#include "DicomProcessing.h"
#include "ITK-5.1/itkBinaryMorphologicalClosingImageFilter.h"
#include "ITK-5.1/itkBinaryShapeKeepNObjectsImageFilter.h"
#include "ITK-5.1/itkSubtractImageFilter.h"
#include "ITK-5.1/itkAffineTransform.h"
#include "ITK-5.1/itkNearestNeighborInterpolateImageFunction.h"
#include "ITK-5.1/itkMatrix.h"

class ZygomaticArchPointsExtractor : public itk::ProcessObject
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(ZygomaticArchPointsExtractor);
	ZygomaticArchPointsExtractor() { };
	~ZygomaticArchPointsExtractor();

	using Self = ZygomaticArchPointsExtractor;
	using Superclass = Object;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(ZygomaticArchPointsExtractor);

	void SetImage(DicomSeriesType::Pointer image);
	void SetForegroundValue(DicomSeriesType::PixelType value) { this->foregroundValue = value; }
	void Compute();
	void SetNasionPoint(std::vector<int> nasionPoint) { this->nasion = nasionPoint; }

	DicomImage* GetProjection() { return this->projection; }
	DicomImage* GetFilteredProjection() { return this->filteredProjection; }
	DicomImage* GetArchSpacesProjection() { return this->archSpacesProjection; }
	DicomImage* GetArchProjection() { return this->archsProjection; }
	DicomSeries* GetArchImage() { return this->archImage; }

	DicomImage* CreateTopProjection(DicomSeries* image);
	DicomSeriesType::IndexType GetLeft() { return this->leftPoint; }
	DicomSeriesType::IndexType GetRight() { return this->rightPoint; }
	std::vector<DicomSeriesType::IndexType> GetZygomaticPoints() { return this->zygomaticPoints; }

private:
	using ImageType = DicomSeriesType;
	using IndexType = typename ImageType::IndexType;
	using PixelType = typename ImageType::PixelType;

	using ClosingKernelType = itk::FlatStructuringElement<DicomImageType::ImageDimension>;
	using ClosingType = itk::BinaryMorphologicalClosingImageFilter<DicomImageType, DicomImageType, ClosingKernelType>;
	using KeepNType = itk::BinaryShapeKeepNObjectsImageFilter<DicomImageType>;
	using SubtractType = itk::SubtractImageFilter<DicomImageType, DicomImageType>;

	DicomSeries* ExtractZygomaticArchs(DicomImage* projection, DicomSeries* input);
	std::vector<DicomImageType::IndexType> GetCutPointsExternal(DicomImageType::Pointer image);
	std::vector<DicomImageType::IndexType> GetCutPointsBackInternal(DicomImageType::Pointer image);
	std::vector<DicomImageType::IndexType> GetCutPointsFrontInternal(DicomImageType::Pointer image);
	std::vector<DicomSeriesType::IndexType> ExtractZygomaticPoints(DicomSeries* zygomaticImage);

	DicomImage* GetRotatedImage(DicomImage* input);
	DicomSeries* GetRotatedImage3D(DicomSeries* input);

	// consumes
	PixelType foregroundValue = SHRT_MAX;
	std::vector<int> nasion;
	
	// creates and eats
	DicomSeries* image = nullptr;

	// produces
	IndexType leftPoint;
	IndexType rightPoint;
	DicomImage* projection;
	DicomImage* filteredProjection;
	DicomImage* archSpacesProjection;
	DicomSeries* archImage;

	DicomImage* archsProjection;
	std::vector<IndexType> zygomaticPoints;

	double rotateAngle = 0; // in degrees
	itk::Matrix<double, 2U, 2U> rotateMatrix;
	itk::Matrix<double, 3U, 3U> rotateMatrix3D;
};

#endif ZYGOMATICARCH_HEADER
