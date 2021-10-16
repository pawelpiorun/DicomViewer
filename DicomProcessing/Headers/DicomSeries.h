#pragma once
#include "itkCommon.h"
#include "ITK-5.1/itkGDCMSeriesFileNames.h"
#include "ITK-5.1/itkRescaleIntensityImageFilter.h"

#include "Converters.h"
#include <vtk-9.0/vtkSmartPointer.h>
#include <vtk-9.0/vtkImageData.h>

typedef itk::ImageSeriesReader<DicomSeriesType> DicomSeriesReaderType;
typedef itk::ImageFileWriter<DicomSeriesType> DicomSeriesWriterType;

class DicomSeries
{
public:
	DicomSeries() { }
	void InitializeReader();
	void DisposeReader();
	bool Open(const char* folderPath);
	void Save(const char* folderPath);
	void SaveVTI(const char* filePath);

	itk::SmartPointer<DicomSeriesType> GetItkImageData() { return dicomSeries; }
	void SetItkImageData(itk::SmartPointer<DicomSeriesType> imageData) { this->dicomSeries = imageData; }
	void Print(std::ostream& os);

	vtkSmartPointer<vtkImageData> GetVtkImageData(bool requiresUpdate = false);
	double GetReaderProgress();

	std::string GetPatientId() { return patientId; }
	std::string GetImageId() { return imageId; }
	std::string GetPatientName() { return patientName; }
	std::string GetStudyDate() { return studyDate; }
	std::string GetAcquisitionDate() { return acquisitionDate; }
	std::string GetFolderName() { return folderName; }
	std::string GetFolderPath() { return folderPath; }

	~DicomSeries();

private:
	std::string folderPath;
	std::string folderName;
	std::string patientId;
	std::string imageId;
	std::string patientName;
	std::string studyDate;
	std::string acquisitionDate;

	vtkSmartPointer<vtkImageData> imageData;
	itk::SmartPointer<DicomSeriesReaderType> reader;
	itk::SmartPointer<DicomSeriesType> dicomSeries;
	itk::SmartPointer<itk::GDCMImageIO> gdcmIO;
};

