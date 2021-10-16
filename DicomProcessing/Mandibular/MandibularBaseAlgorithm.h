#pragma once
#include "DicomSeries.h"
#include "DicomProcessing.h"
#include "MandibularBoneToCondylesAlgorithm.h"
#include "ITK-5.1/itkMedianImageFilter.h"
#include "NasionPointExtractor.h"
#include "ZygomaticArchPointsExtractor.h"

class MandibularBaseAlgorithm
	: public itk::ImageToImageFilter<DicomSeriesType, DicomSeriesType>
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(MandibularBaseAlgorithm);
	itkTypeMacro(MandibularBaseAlgorithm, ImageToImageFilter);

	using Self = MandibularBaseAlgorithm;
	using Superclass = ImageToImageFilter<DicomSeriesType, DicomSeriesType>;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	using ImageType = DicomSeriesType;
	using PixelType = typename ImageType::PixelType;

	MandibularBaseAlgorithm();
	~MandibularBaseAlgorithm();
	int Run(DicomSeries* dicomSeries, bool keepOriginalImage = false);
	long GetVoxelCount();
	float GetProgress();
	bool GetSuccess() { return this->success; }
	char* GetErrorMessage() { return this->errorMessage; }

	//std::vector<int> GetLeftCondylePosition();
	//std::vector<int> GetRightCondylePosition();
	//std::vector<double> GetLeftCondyleRotation();
	//std::vector<double> GetRightCondyleRotation();
	void GetLeftCondylePositionEX(double** arr, int* size);
	void GetRightCondylePositionEX(double** arr, int* size);
	void GetCondylesDifferenceEX(double** arr, int* size);
	double GetCondylesDistancePx() { return this->condylesDistancePx; }
	double GetCondylesDistance() { return this->condylesDistance; }

	void GetNasionPointEX(double** arr, int* size);
	void GetLeft2NasionDifferenceEx(double** arr, int* size);
	void GetRight2NasionDifferenceEx(double** arr, int* size);
	double GetLeft2NasionDistancePx() { return this->left2NasionDistancePx; }
	double GetLeft2NasionDistance() { return this->left2NasionDistance; }
	double GetRight2NasionDistancePx() { return this->right2NasionDistancePx; }
	double GetRight2NasionDistance() { return this->right2NasionDistance; }

	void GetLeftZygomaticPointEx(double** arr, int* size);
	void GetRightZygomaticPointEx(double** arr, int* size);

	void GetCenter(double** arr, int* size) { GetPoint(center, arr, size); }
	void GetLeftToCenterDifferenceEx(double** arr, int* size) { GetPoint(left2CenterDiff, arr, size); }
	void GetRightToCenterDifferenceEx(double** arr, int* size) { GetPoint(right2CenterDiff, arr, size); }
	double GetLeftToCenterDistance() { return left2CenterDistance; }
	double GetRightToCenterDistance() { return right2CenterDistance; }

	void GetImageSpacing(double** arr, int* size) { GetPoint(spacing, arr, size); }
	void GetImageOrigin(double** arr, int* size) { GetPoint(origin, arr, size); }

	void GetLeftCondyleFinal(double** arr, int* size) { GetPoint(leftCondyleFinal, arr, size); }
	void GetRightCondyleFinal(double** arr, int* size) { GetPoint(rightCondyleFinal, arr, size); }

protected:
	std::vector<ImageSource<ImageType>::Pointer> GetProcessObjects();
	virtual ImageSource<ImageType>::Pointer GetLastPipelineObject() = 0; // that makes the class abstract
	void GenerateData() override;
	bool success;
	char* errorMessage;
	bool endWithZygomaticArchs = false;

	// intermediates
	DicomImage* leftLinesImage = nullptr;
	DicomImage* rightLinesImage = nullptr;
	DicomSeries* input = nullptr;
	DicomImage* upperSkullProjection = nullptr;
	DicomImage* topCutProjection = nullptr;
	DicomImage* topCutFilteredProjection = nullptr;
	DicomImage* archSpacesProjection = nullptr;
	DicomImage* archsProjection = nullptr;

private:
	void CalculateCondylesInfo();
	int GetMeanZCoord(ImageType* input);
	int GetMeanZCoord(std::vector<ImageType::IndexType> input);
	void RemovePointsAbove(ImageType* input, int maxZ);
	std::vector<ImageType::IndexType> GetPointsBelowZ(ImageType* input, std::vector<std::vector<int>> boundingBoxXYZ, int maxZ);

	itk::SmartPointer<itk::ProgressAccumulator> progressAccumulator;
	const PixelType _selectionValue = SHRT_MAX;

	void GetPoint(std::vector<int> pt, double** targetArr, int* size);
	void GetPoint(std::vector<double> pt, double** targetArr, int* size);
	std::vector<int> leftCondylePos = std::vector<int>(3);
	std::vector<int> rightCondylePos = std::vector<int>(3);
	std::vector<int> condylesPosDiff = std::vector<int>(3);
	double condylesDistancePx = -1;
	double condylesDistance = -1;

	std::vector<double> leftCondyleRot = std::vector<double>(3);
	std::vector<double> rightCondyleRot = std::vector<double>(3);
	int voxelCount;

	using MedianFilterType = itk::MedianImageFilter<DicomSeriesType, DicomSeriesType>;
	using ThresholdType = itk::BinaryThresholdImageFilter<DicomSeriesType, DicomSeriesType>;
	using SubtractFilterType = itk::SubtractImageFilter<DicomSeriesType, DicomSeriesType, DicomSeriesType>;
	using DilateKernelType = itk::FlatStructuringElement<DicomSeriesType::ImageDimension>;
	using DilateType = itk::BinaryDilateImageFilter<DicomSeriesType, DicomSeriesType, DilateKernelType>;
	using ConnectedThresholdType = itk::ConnectedThresholdImageFilter<DicomSeriesType, DicomSeriesType>;
	using LabelStatisticsFilter = itk::LabelStatisticsImageFilter<DicomSeriesType, DicomSeriesType>;
	using KeepNFilterType = itk::BinaryShapeKeepNObjectsImageFilter<DicomSeriesType>;
	using LabelPixelType = LabelStatisticsFilter::LabelPixelType;
	using ProcessObjectType = itk::ImageSource<ImageType>;
	using MandibularBoneToCondylesType = MandibularBoneToCondylesAlgorithm;

	MedianFilterType::Pointer medianFilter;
	ThresholdType::Pointer teethThreshold;
	DilateType::Pointer teethDilate;
	KeepNFilterType::Pointer keepNFilter;
	ThresholdType::Pointer generalThreshold;
	SubtractFilterType::Pointer subtract;
	ProcessObject::Pointer pointsSelection;
	ConnectedThresholdType::Pointer connectedThreshold;
	SubtractFilterType::Pointer mandibulaSubtract;
	LabelStatisticsFilter::Pointer labelStatistics;
	MandibularBoneToCondylesType::Pointer boneToCondyles;

	NasionPointExtractor::Pointer nasionPointExtractor;
	std::vector<int> nasionPoint = std::vector<int>(3);
	std::vector<int> left2NasionDiff = std::vector<int>(3);
	std::vector<int> right2NasionDiff = std::vector<int>(3);
	double left2NasionDistancePx = 0;
	double left2NasionDistance = 0;
	double right2NasionDistancePx = 0;
	double right2NasionDistance = 0;

	std::vector<int> leftZygomaticPoint;
	std::vector<int> rightZygomaticPoint;

	std::vector<int> center = std::vector<int>(3);
	std::vector<int> left2CenterDiff = std::vector<int>(3);
	std::vector<int> right2CenterDiff = std::vector<int>(3);

	double left2CenterDistance = 0;
	double right2CenterDistance = 0;

	std::vector<double> spacing = std::vector<double>(3);
	std::vector<double> origin = std::vector<double>(3);

	// in new physical world
	std::vector<double> leftCondyleFinal = std::vector<double>(3);
	std::vector<double> rightCondyleFinal = std::vector<double>(3);
};