#include "DicomImage.h"

bool DicomImage::Open(const char * filePath)
{
	auto reader = DicomImageReaderType::New();
	reader->SetFileName(filePath);

	gdcmIO = itk::GDCMImageIO::New();
	reader->SetImageIO(gdcmIO);

	try
	{
		reader->Update();
		dicomImage = reader->GetOutput();
	}
	catch (itk::ExceptionObject e)
	{
		std::cout << e.GetDescription();
		throw e;
		return false;
	}

	gdcmIO->GetValueFromTag("0010|0010", patientName);
	gdcmIO->GetValueFromTag("0010|0020", patientId);
	gdcmIO->GetValueFromTag("0008|0020", studyDate);
	gdcmIO->GetValueFromTag("0008|0022", acquisitionDate);
	this->filePath = filePath;
	auto lastSlash = this->filePath.find_last_of('\\');
	auto lastDot = this->filePath.find_last_of('.');
	this->fileName = this->filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);

	return true;
}

void DicomImage::Save(const char * filePath, bool withSpacing)
{
	auto writer = DicomImageWriterType::New();
	writer->SetFileName(filePath);
	if (!withSpacing)
	{
		writer->SetInput(this->dicomImage);
		writer->Update();
	}
	else
	{
		auto resampled = CreateResampled();
		writer->SetInput(resampled);
		writer->Update();
	}

	this->filePath = filePath;
	auto lastSlash = this->filePath.find_last_of('\\');
	auto lastDot = this->filePath.find_last_of('.');
	this->fileName = this->filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
}

void DicomImage::SaveCompressed(const char * filePath, bool withSpacing)
{
	using GrayImageWriterType = itk::ImageFileWriter<GrayImageType>;
	auto writer = GrayImageWriterType::New();
	writer->SetFileName(filePath);
	if (!withSpacing)
	{
		auto compressed = Converters::Convert<DicomImageType, GrayImageType>(dicomImage);
		writer->SetInput(compressed);
		writer->Update();
	}
	else
	{
		auto resampled = CreateResampled();
		auto compressed = Converters::Convert<DicomImageType, GrayImageType>(resampled);
		writer->SetInput(compressed);
		writer->Update();
	}

	//this->filePath = filePath;
	//auto lastSlash = this->filePath.find_last_of('\\');
	//auto lastDot = this->filePath.find_last_of('.');
	//this->fileName = this->filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
}

signed short* DicomImage::GetData()
{
	auto image = dicomImage;
	auto region = image->GetLargestPossibleRegion();
	auto size = region.GetSize();

	width = size[0];
	height = size[1];
	auto imageSize = width * height;

	this->dataBuffer = new signed short[imageSize];

	auto ptr = image->GetBufferPointer();
	for (int i = 0; i < imageSize; i++)
	{
		dataBuffer[i] = *ptr;
		ptr++;
	}

	return dataBuffer;
}

BYTE* DicomImage::GetImageBytes()
{
	auto output = Converters::Rescale<DicomImageType, GrayImageType>(dicomImage);
	auto region = output->GetLargestPossibleRegion();
	auto size = region.GetSize();

	width = size[0];
	height = size[1];
	auto imageSize = width * height;

	this->imageBuffer = new unsigned char[imageSize];

	auto ptr = output->GetBufferPointer();
	for (int i = 0; i < imageSize; i++)
	{
		imageBuffer[i] = *ptr;
		ptr++;
	}

	return imageBuffer;
}

void DicomImage::Resample()
{
	auto resampled = CreateResampled();
	SetItkDicomImage(resampled);
}

void DicomImage::Swap()
{
	auto sourceImage = GetItkDicomImage();
	auto sourceRegion = sourceImage->GetLargestPossibleRegion();
	auto sourceSize = sourceRegion.GetSize();

	auto dicomImage = DicomImageType::New();
	dicomImage->SetRegions(sourceRegion);
	dicomImage->Allocate();
	dicomImage->FillBuffer(0);
	dicomImage->SetSpacing(sourceImage->GetSpacing());

	using ConstIteratorType = itk::ImageRegionConstIterator<DicomImageType>;
	using IteratorType = itk::ImageRegionIterator<DicomImageType>;
	auto region = dicomImage->GetLargestPossibleRegion();
	auto iterator = IteratorType(dicomImage, region);
	auto sourceIt = ConstIteratorType(sourceImage, sourceRegion);
	iterator.GoToBegin();
	sourceIt.GoToBegin();

	auto width = sourceSize[0];
	auto height = sourceSize[1];
	while (!iterator.IsAtEnd())
	{
		auto val = sourceIt.Value();
		auto in = sourceIt.GetIndex();
		DicomImageType::IndexType swappedIndex = { width - in[0] - 1, height - in[1] - 1 };
		dicomImage->SetPixel(swappedIndex, val);

		++iterator;
		++sourceIt;
	}

	SetItkDicomImage(dicomImage);
}

itk::SmartPointer<DicomImageType> DicomImage::CreateResampled()
{
	using ScalarType = double;
	DicomImageType::RegionType  region = dicomImage->GetLargestPossibleRegion();
	DicomImageType::SizeType    size = region.GetSize();
	DicomImageType::SpacingType spacing = dicomImage->GetSpacing();

	auto scale = spacing[1] / spacing[0];
	DicomImageType::SizeType    newSize = { size[0], size[1] * scale };
	DicomImageType::SpacingType newSpacing = DicomImageType::SpacingType();
	newSpacing[0] = spacing[0];
	newSpacing[1] = spacing[0]; // same

	using InterpolateType = itk::NearestNeighborInterpolateImageFunction<DicomImageType, ScalarType>;
	InterpolateType::Pointer interpolator = InterpolateType::New();

	resampleFilter = ResampleFilterType::New();

	resampleFilter->SetInput(dicomImage);
	resampleFilter->SetInterpolator(interpolator);
	resampleFilter->SetSize(newSize);
	resampleFilter->SetOutputSpacing(newSpacing);
	resampleFilter->Update();
	return resampleFilter->GetOutput();
}

void DicomImage::CopyInformation(DicomSeries* dicomSeries)
{
	this->acquisitionDate = dicomSeries->GetAcquisitionDate();
	this->studyDate = dicomSeries->GetStudyDate();
	this->patientId = dicomSeries->GetPatientId();
	this->patientName = dicomSeries->GetPatientName();
	this->imageId = dicomSeries->GetImageId();
}

void DicomImage::CopyInformation(DicomImage* dicomImage)
{
	this->acquisitionDate = dicomImage->GetAcquisitionDate();
	this->studyDate = dicomImage->GetStudyDate();
	this->patientId = dicomImage->GetPatientId();
	this->patientName = dicomImage->GetPatientName();
	this->imageId = dicomImage->GetImageId();
}


void DicomImage::SetItkDicomImage(itk::SmartPointer<DicomImageType> image)
{
	this->dicomImage = image;
	auto size = image->GetLargestPossibleRegion().GetSize();
	width = size[0];
	height = size[1];
}

void DicomImage::Print(std::ostream& os)
{
	gdcmIO->Print(os);
}

DicomImage::~DicomImage()
{
	dicomImage = nullptr;
	dataBuffer = nullptr;
	imageBuffer = nullptr;
}