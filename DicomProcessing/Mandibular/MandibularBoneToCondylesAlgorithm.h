#pragma once
#include "DicomImage.h"
#include "DicomSeries.h"
#include "DicomProcessing.h"
#include "BinaryImageToSymmetryPlane.h"
#include "RamusContoursToRamusLineAlgorithm.h"
#include "RamusContoursToNotchLineAlgorithm.h"
#include <itkBinaryMorphologicalClosingImageFilter.h>

class MandibularBoneToCondylesAlgorithm
	: public itk::ImageToImageFilter<DicomSeriesType, DicomSeriesType>
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(MandibularBoneToCondylesAlgorithm);

	using Self = MandibularBoneToCondylesAlgorithm;
	using Superclass = ImageToImageFilter<DicomSeriesType, DicomSeriesType>;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(MandibularBoneToCondylesAlgorithm);
	itkTypeMacro(MandibularBoneToCondylesAlgorithm, ImageToImageFilter);

	using ImageType = DicomSeriesType;
	using PixelType = typename ImageType::PixelType;

	MandibularBoneToCondylesAlgorithm();
	~MandibularBoneToCondylesAlgorithm();

	void SetForegroundValue(ImageType::PixelType value) { this->_foregroundValue = value; }
	void SetBackgroundValue(ImageType::PixelType value) { this->_backgroundValue = value; }
	DicomImage* GetLeftLinesImage() { return this->leftLinesImage; }
	DicomImage* GetRightLinesImage() { return this->rightLinesImage; }

	ImageType::IndexType GetLeftCondyleCenter() { return this->leftCondyleCenter; }
	ImageType::IndexType GetRightCondyleCenter() { return this->rightCondyleCenter; }

protected:
	void GenerateData() override;

private:
	enum CondyleType
	{
		Right,
		Left
	};
	std::vector<ImageType::IndexType> leftCondylePoints;
	std::vector<ImageType::IndexType> rightCondylePoints;
	ImageType::IndexType leftCondyleCenter;
	ImageType::IndexType rightCondyleCenter;

	ImageType::IndexType GetPointsGeometricalCenter(std::vector<ImageType::IndexType> points);
	std::vector<ImageType::IndexType> GetCondyle(ImageType::Pointer image, CondyleType condyleType);
	DicomImage* GetProjection(ImageType::Pointer image, std::vector<ImageType::IndexType> points, CondyleType condyleType);
	DicomImage* GetLeftContours(DicomImage* image);
	DicomImage* GetTopContours(DicomImage* image);
	std::vector<double> GetNotchRamusLineWithMinimalSection(std::vector<double> notchLine, std::vector<ImageType::IndexType> points);

	void MandibularBoneToCondylesAlgorithm::SaveLineImage(DicomImage* image, std::vector<double> line);
	void MandibularBoneToCondylesAlgorithm::SaveAllLinesImage(
		DicomImage* image, std::vector<std::vector<double>> lines);
	double GetArea(std::vector<ImageType::IndexType> points);

	std::vector<std::vector<int>> boundingBoxXYZ;
	ImageType::RegionType GetBoundedRegion();
	ImageType::RegionType boundingRegion;
	ImageType::IndexType _mediumPoint;
	ImageType::PixelType _foregroundValue = SHRT_MAX;
	ImageType::PixelType _backgroundValue = SHRT_MIN;
	const int closingIterations = 6;
	const int closingFilterRadius = 3;

	using KeepNFilterType = itk::BinaryShapeKeepNObjectsImageFilter<DicomSeriesType>;
	using RamusLineType = RamusContoursToRamusLineAlgorithm;
	using NotchLineType = RamusContoursToNotchLineAlgorithm;
	using ClosingKernelType = itk::FlatStructuringElement<DicomSeriesType::ImageDimension>;
	using ClosingFilterType = itk::BinaryMorphologicalClosingImageFilter<ImageType, ImageType, ClosingKernelType>;

	ClosingFilterType::Pointer closingFilter;
	KeepNFilterType::Pointer keepNFilter;
	RamusLineType::Pointer leftRamusLineExtractor;
	RamusLineType::Pointer rightRamusLineExtractor;
	NotchLineType::Pointer leftNotchLineExtractor;
	NotchLineType::Pointer rightNotchLineExtractor;

	DicomImage* leftLinesImage = nullptr;
	DicomImage* rightLinesImage = nullptr;
};