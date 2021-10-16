#pragma once
#include "DicomSeries.h"
#include "DicomImage.h"
#include "RgbImage.h"
#include "DicomProcessing.h"
#include "ITK-5.1/itkBinaryThinningImageFilter3D.h"
#include "ITK-5.1/itkMedianImageFilter.h"
#include "AirVolumeSkeletonSimplificationFilter.h"
#include "AirVolumeSkeletonPartitioningFilter.h"
#include "ITK-5.1/itkBinaryMorphologicalClosingImageFilter.h"

#define SideProjection 0
#define FrontProjection 1
#define TopProjection 2

class AirVolumeAlgorithm
	: public itk::ImageToImageFilter<DicomSeriesType, DicomSeriesType>
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(AirVolumeAlgorithm);
	itkTypeMacro(AirVolumeAlgorithm, ImageToImageFilter);

	using Self = AirVolumeAlgorithm;
	using Superclass = ImageToImageFilter<DicomSeriesType, DicomSeriesType>;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	using ImageType = DicomSeriesType;
	using PixelType = typename ImageType::PixelType;

	AirVolumeAlgorithm();
	~AirVolumeAlgorithm();
	void Run(DicomSeries* dicomSeries, bool keepOriginalImage = true);
	long GetVoxelCount() { return voxelCount; };
	double GetCalculatedVolume() { return calculatedVolume; };
	double GetCalculatedPharynx() { return calculatedPharynx; };
	double GetCalculatedNasalCavity() { return calculatedNasalCavity; };
	double GetMinimalCrossSection() { return minimalCrossSection; };
	double GetMinimalCrossSection2() { return minimalCrossSection2; }
	double GetMinimalCrossSection3() { return minimalCrossSection3; }
	double GetMinimalCrossSection4() { return minimalCrossSection4; }
	float GetProgress();

	void DumpIntermediates(const char* folderPath);
	void GetCrossSectionsProfile(double** arr, int* size);
	void GetCrossSectionsProfile2(double** arr, int* size);
	void GetCrossSectionsProfile3(double** arr, int* size);
	void GetCrossSectionsProfile4(double** arr, int* size);
	void GetCrossSectionCoord(int index, int** arr, int* size);
	void GetCrossSectionNormal(int index, double** arr, int* size);

protected:
	void GenerateData() override;

private:
	DicomImage* GetAirVolumeLowerHalfProjection(ImageType::Pointer airVolume, bool right = false);
	DicomImage* GetAirVolumeProjection(ImageType::Pointer airVolume, int projectionType = SideProjection);
	std::vector<DicomImageType::IndexType> GetEpiglottisCutLinePoints(DicomImage* projection);
	std::vector<DicomImageType::IndexType> GetOroPharynxCutLinePoints(DicomImage* projection);
	std::vector<DicomImageType::IndexType> GetSinusCutLinePoints(DicomImage* projection, int mediumZ);
	std::vector<DicomImageType::IndexType> SwapPoints(DicomImage* projection, std::vector<DicomImageType::IndexType> points);
	std::vector<double> CreateLine(DicomImageType::IndexType first, DicomImageType::IndexType second);
	void RemoveVoxelsBelowEpiglottis(ImageType::Pointer image, std::vector<double> epiglottisCutLine);
	void RemoveOroPharynxVoxels(ImageType::Pointer image, std::vector<DicomImageType::IndexType> oroPharynxCutLine);
	void RemoveSinusesVoxels(ImageType::Pointer image, std::vector<double> sinusCutLine, std::vector<DicomImageType::IndexType> sinusCutPoints);
	void OpenSinusesVoxels(ImageType::Pointer image, std::vector<double> sinusCutLine, std::vector<DicomImageType::IndexType> sinusCutPoints);
	void SaveLineImage(DicomImage* image, std::vector<double> line, const char* outputPath);
	long GetPharynxVoxelCount(DicomSeriesType::Pointer image, DicomSeriesType::IndexType splitPoint, std::vector<double> splitPlane);

	DicomSeries* input = nullptr;
	DicomImage* sideProjection = nullptr;
	DicomImage* frontProjection = nullptr;
	//DicomImage* lowerHalfSideProjection = nullptr;
	DicomImage* skeletonSideProjection = nullptr;
	DicomImage* skeletonFrontProjection = nullptr;
	DicomImage* simplifiedSkeletonSideProjection = nullptr;
	DicomImage* simplifiedSkeletonFrontProjection = nullptr;
	DicomImage* minimalPathsFront = nullptr;
	DicomImage* minimalPathsSide = nullptr;
	RgbImage* frontCrossSection = nullptr;
	RgbImage* sideCrossSection = nullptr;
	std::vector<DicomSeriesType::IndexType> minimalPathsVoxels;

	itk::SmartPointer<itk::ProgressAccumulator> progressAccumulator;
	long voxelCount = 0;
	long pharynxVoxelCount = 0;
	long nasalCavityVoxelCount = 0;
	double calculatedVolume;
	double calculatedPharynx;
	double calculatedNasalCavity;
	double minimalCrossSection;
	double minimalCrossSection2;
	double minimalCrossSection3;
	double minimalCrossSection4;
	std::vector<double> minimalCrossSections;
	std::vector<double> minimalCrossSections2;
	std::vector<double> minimalCrossSections3;
	std::vector<double> minimalCrossSections4;
	std::vector<ImageType::IndexType> minimalCrossSectionsCoords;
	std::vector<std::vector<double>> minimalCrossSectionsNormals;
	const PixelType _selectionValue = SHRT_MAX;

	using MedianFilterType = itk::MedianImageFilter<DicomSeriesType, DicomSeriesType>;
	using ThresholdType = itk::BinaryThresholdImageFilter<DicomSeriesType, DicomSeriesType>;
	using ErodeKernelType = itk::FlatStructuringElement<DicomSeriesType::ImageDimension>;
	using ErodeType = itk::BinaryErodeImageFilter<DicomSeriesType, DicomSeriesType, ErodeKernelType>;
	using ConnectedThresholdType = itk::ConnectedThresholdImageFilter<DicomSeriesType, DicomSeriesType>;
	using DilateKernelType = itk::FlatStructuringElement<DicomSeriesType::ImageDimension>;
	using DilateType = itk::BinaryDilateImageFilter<DicomSeriesType, DicomSeriesType, DilateKernelType>;
	using SubtractFilterType = itk::SubtractImageFilter<DicomSeriesType, DicomSeriesType, DicomSeriesType>;
	using KeepNFilterType = itk::BinaryShapeKeepNObjectsImageFilter<DicomSeriesType>;
	using LabelStatisticsFilter = itk::LabelStatisticsImageFilter<DicomSeriesType, DicomSeriesType>;
	using LabelPixelType = LabelStatisticsFilter::LabelPixelType;
	using SkeletonizeFilter = BinaryThinningImageFilter3D;
	using SkeletonSimplifyFilter = AirVolumeSkeletonSimplificationFilter;
	using VolumePartitioningFilter = AirVolumeSkeletonPartitioningFilter;
	using ClosingKernelType = itk::FlatStructuringElement<DicomSeriesType::ImageDimension>;
	using ClosingFilter = itk::BinaryMorphologicalClosingImageFilter<DicomSeriesType, DicomSeriesType, ClosingKernelType>;

	MedianFilterType::Pointer medianFilter;
	ThresholdType::Pointer threshold;
	ErodeType::Pointer erode;
	ConnectedThresholdType::Pointer connectedThreshold;
	DilateType::Pointer dilate;
	SubtractFilterType::Pointer subtract;
	KeepNFilterType::Pointer keepNFilter;
	KeepNFilterType::Pointer keepAfterCutFilter;
	ClosingFilter::Pointer closingFilter;
	LabelStatisticsFilter::Pointer labelStatistics;
	//ErodeType::Pointer erode2;
	//KeepNFilterType::Pointer keepNFilter2;
	//DilateType::Pointer dilate2;
	SkeletonizeFilter::Pointer skeletonizeFilter;
	SkeletonSimplifyFilter::Pointer skeletonSimplify;
	VolumePartitioningFilter::Pointer volumePartitioning;
	KeepNFilterType::Pointer keepAfterPartitioning;

	std::vector<ImageSource<ImageType>::Pointer> GetProcessObjects()
	{
		return std::vector<ImageSource<ImageType>::Pointer>
		{
			medianFilter,
			threshold,
			erode,
			connectedThreshold,
			dilate,
			subtract,
			keepNFilter,
			closingFilter,
			keepAfterCutFilter,
			skeletonizeFilter,
			skeletonSimplify,
			volumePartitioning,
			keepAfterPartitioning
		};
	};
	void SetLastPipelineObject(ImageSource<ImageType>::Pointer object)
	{
		lastPipeline = object;

		auto objects = GetProcessObjects();
		pipelineCount = objects.size();

		auto lastIt = std::find(objects.begin(), objects.end(), lastPipeline);
		lastPipelineIndex = std::distance(objects.begin(), lastIt);
	};
	ImageSource<ImageType>::Pointer lastPipeline;
	int lastPipelineIndex;
	int pipelineCount;

	const int MedianIndex = 0;
	const int ThersholdIndex = 1;
	const int ErodeIndex = 2;
	const int ConnectedThresholdIndex = 3;
	const int DilateIndex = 4;
	const int SubtractIndex = 5;
	const int KeepNFilterIndex = 6;
	const int ClosingIndex = 7;
	const int KeepAfterCutIndex = 8;
	const int SkeletonizeIndex = 9;
	const int SimplifySkeletonIndex = 10;
	const int VolumePartitioningIndex = 11;
	const int KeepAfterPartitioning = 12;
};