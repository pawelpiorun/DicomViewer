#include "DicomSeries.h"
#include "helpers.h"
#include "vtk-9.0/vtkXMLImageDataWriter.h"

// TODO - enable loading path with polish letters
bool DicomSeries::Open(const char * folderPath)
{
	gdcmIO = itk::GDCMImageIO::New();
	reader->SetImageIO(gdcmIO);

	// get name generator by series date (0008|0021)
	auto nameGenerator = itk::GDCMSeriesFileNames::New();
	nameGenerator->SetUseSeriesDetails(true);
	nameGenerator->AddSeriesRestriction("0008|0021");

	nameGenerator->SetDirectory(folderPath);
	this->folderPath = folderPath;
	auto lastSlash = this->folderPath.find_last_of('\\');
	this->folderName = this->folderPath.substr(lastSlash + 1);

	// get all series identifiers
	const std::vector<std::string> & seriesUID = nameGenerator->GetSeriesUIDs();
	if (seriesUID.size() == 0) return false;

	auto seriesItr = seriesUID.begin();
	auto seriesEnd = seriesUID.end();
	while (seriesItr != seriesEnd)
	{
		std::cout << seriesItr->c_str() << std::endl;
		++seriesItr;
	}

	// get series identifier
	std::string seriesIdentifier = seriesUID.begin()->c_str();

	// get and pass file names to reader
	using FileNamesContainer = std::vector< std::string >;
	FileNamesContainer fileNames;
	fileNames = nameGenerator->GetFileNames(seriesIdentifier);
	reader->SetFileNames(fileNames);

	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject e)
	{
		std::cout << e.GetDescription() << std::endl;
		return false;
	}

	gdcmIO->GetValueFromTag("0010|0010", patientName);
	gdcmIO->GetValueFromTag("0010|0020", patientId);
	gdcmIO->GetValueFromTag("0020|000E", imageId);
	gdcmIO->GetValueFromTag("0008|0020", studyDate);
	gdcmIO->GetValueFromTag("0008|0022", acquisitionDate);

	patientName = helpers::trim(patientName);

	dicomSeries = reader->GetOutput();
	return true;
}
void DicomSeries::Save(const char* folderPath)
{
	// TODO ?
	//auto writer = DicomSeriesWriterType::New();
	//writer->SetInput(this->dicomSeries);
	//writer->SetFileName(folderPath);
	//writer->Update();
}

void DicomSeries::SaveVTI(const char* filePath)
{
	using ModelWriterType = vtkXMLImageDataWriter;
	auto writer = ModelWriterType::New();
	writer->SetFileName(filePath);

	auto input = this->GetVtkImageData();
	writer->SetInputData(input);
	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}
}

void DicomSeries::InitializeReader()
{
	DisposeReader();
	this->reader = DicomSeriesReaderType::New();
}
double DicomSeries::GetReaderProgress()
{
	return reader->GetProgress();
}
void DicomSeries::DisposeReader()
{
	reader = nullptr;
}

vtkSmartPointer<vtkImageData> DicomSeries::GetVtkImageData(bool requiresUpdate)
{
	if (imageData == nullptr || requiresUpdate)
		imageData = Converters::itkToVtk<DicomSeriesType>(dicomSeries);

	return imageData;
}

void DicomSeries::Print(std::ostream& os)
{
	this->gdcmIO->Print(os);
	this->dicomSeries->Print(os);
}

DicomSeries::~DicomSeries()
{
	DisposeReader();
	dicomSeries = nullptr;
}
