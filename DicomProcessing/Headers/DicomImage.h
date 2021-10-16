#pragma once
#include "itkCommon.h"
#include "Converters.h"

#include "ITK-5.1/itkNearestNeighborInterpolateImageFunction.h"
#include "ITK-5.1/itkResampleImageFilter.h"

#include "DicomSeries.h"

typedef itk::ImageFileReader<DicomImageType> DicomImageReaderType;
typedef itk::ImageFileWriter<DicomImageType> DicomImageWriterType;

class DicomImage
{
public:
	DicomImage() { }
	~DicomImage();
	bool Open(const char * filePath);
	void Save(const char* filePath, bool withSpacing = false);
	void SaveCompressed(const char* filePath, bool withSpacing = false);

	void SetItkDicomImage(itk::SmartPointer<DicomImageType> image);
	itk::SmartPointer<DicomImageType> GetItkDicomImage() { return dicomImage; }
	void Print(std::ostream& os);

	signed short* GetData();
	BYTE* GetImageBytes();

	int GetHeight() { return height; }
	int GetWidth() { return width; }
	std::string GetPatientId() { return patientId; }
	std::string GetPatientName() { return patientName; }
	std::string GetStudyDate() { return studyDate; }
	std::string GetAcquisitionDate() { return acquisitionDate; }
	std::string GetFilePath() { return filePath; }
	std::string GetFileName() { return fileName; }
	std::string GetImageId() { return imageId; }
	void SetFileName(const char* fileName) { this->fileName = fileName; }

	void Resample();
	void Swap();
	void CopyInformation(DicomSeries* dicomSeries);
	void CopyInformation(DicomImage* dicomImage);

private:
	itk::SmartPointer<DicomImageType> CreateResampled();

	itk::SmartPointer<itk::GDCMImageIO> gdcmIO;
	itk::SmartPointer<DicomImageType> dicomImage;
	signed short* dataBuffer;
	BYTE* imageBuffer;
	int width;
	int height;

	std::string filePath;
	std::string fileName;
	std::string patientId;
	std::string imageId;
	std::string patientName;
	std::string studyDate;
	std::string acquisitionDate;

	using ResampleFilterType = itk::ResampleImageFilter<DicomImageType, DicomImageType>;
	ResampleFilterType::Pointer resampleFilter;
};