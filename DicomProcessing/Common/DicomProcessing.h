#pragma once
#include "DicomImage.h"
#include "DicomSeries.h"
#include "ITK-5.1/itkBinaryErodeImageFilter.h"
#include "ITK-5.1/itkBinaryDilateImageFilter.h"
#include "ITK-5.1/itkFlatStructuringElement.h"
#include "ITK-5.1/itkBinaryBallStructuringElement.h"
#include "ITK-5.1/itkBinaryThresholdImageFilter.h"
#include "ITK-5.1/itkConnectedThresholdImageFilter.h"
#include "ITK-5.1/itkSubtractImageFilter.h"
#include "ITK-5.1/itkWatershedImageFilter.h"
#include "ITK-5.1/itkBinaryShapeKeepNObjectsImageFilter.h"
#include "ITK-5.1/itkStatisticsImageFilter.h"
#include "ITK-5.1/itkLabelStatisticsImageFilter.h"
#include "ITK-5.1/itkImageDuplicator.h"

typedef itk::Image<itk::IdentifierType, 3U> LabeledSeriesType;

namespace DicomProcessing
{
	itk::SmartPointer <DicomSeriesType> ThresholdBetween(DicomSeries* dicomSeries, short lower, short upper);
	itk::SmartPointer <DicomSeriesType> ThresholdBelow(DicomSeries* dicomSeries, short thresholdValue);
	itk::SmartPointer <DicomSeriesType> ConnectedThreshold(DicomSeries* dicomSeries, short lower, short upper);
	itk::SmartPointer<DicomSeriesType> Dilate(itk::SmartPointer<DicomSeriesType> input, int radiusValue = 1, int iterations = 1);
	itk::SmartPointer<DicomSeriesType> Erode(itk::SmartPointer<DicomSeriesType> input, int radiusValue = 1, int iterations = 1);
	itk::SmartPointer<DicomSeriesType> GetExternalAir(itk::SmartPointer<DicomSeriesType> image);
	std::vector<DicomSeriesType::IndexType> GetCorners(itk::SmartPointer<DicomSeriesType> image);
	itk::SmartPointer<DicomSeriesType> Subtract(itk::SmartPointer<DicomSeriesType> minuend, itk::SmartPointer<DicomSeriesType> subtrahend);
	itk::SmartPointer<LabeledSeriesType> WatershedSegmentation(itk::SmartPointer<DicomSeriesType> image, short threshold, double level);
	//void WriteStatistics(itk::SmartPointer<DicomSeriesType> image);
	DicomSeriesType::IndexType GetMediumPoint(DicomSeriesType::Pointer image, DicomSeriesType::PixelType _foregroundValue);
	DicomSeriesType::IndexType GetMediumPointBelowZ(DicomSeriesType::Pointer image, DicomSeriesType::PixelType _foregroundValue, int aboveZ);
	DicomImageType::IndexType GetMediumPoint(DicomImageType::Pointer image, DicomImageType::PixelType _foregroundValue);

	template <typename T>
	itk::SmartPointer<T> Copy(itk::SmartPointer<T> image);

	int GetVoxelCount(DicomSeriesType* image, DicomSeriesType::PixelType value);
	std::vector<std::vector<int>> GetSelectionBoundingBoxXYZ(DicomSeriesType* image, DicomSeriesType::PixelType selectionValue);

	itk::SmartPointer<DicomImageType> BinaryInverse(itk::SmartPointer<DicomImageType> image, DicomImageType::PixelType foregroundValue = SHRT_MAX);
	
	double* CalculateTransformationMatrix(
		double* sourcePoints, double* sourceCenter,
		double* targetPoints, double* targetCenter, int count = 3);
}