#include "GrayImage.h"

GrayImage::GrayImage()
{

}

void GrayImage::Open(const char* imagePath)
{
	const auto path = itk::SimpleDataObjectDecorator<std::string>::New();
	path->Set(imagePath);

	auto reader = GrayImageReaderType::New();
	reader->SetFileName(path);

	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject err)
	{
		std::cout << err.GetDescription();
		throw err;
	}

	image = reader->GetOutput();
	auto region = image->GetLargestPossibleRegion();
	auto size = region.GetSize();

	width = size[0];
	height = size[1];
	auto imageSize = width * height * 3;

	this->data = image->GetBufferPointer();
}

void GrayImage::SetItkImage(itk::SmartPointer<GrayImageType> image)
{
	this->image = image;
	auto size = image->GetLargestPossibleRegion().GetSize();
	this->width = size[0];
	this->height = size[1];
	if (data != nullptr)
	{
		delete data;
		data = nullptr;
	}
	data = new unsigned char[width * height * 3];
}

BYTE* GrayImage::GetData()
{
	return data;
}

int GrayImage::GetHeight()
{
	return this->height;
}

int GrayImage::GetWidth()
{
	return this->width;
}

GrayImage::~GrayImage()
{
	if (this->data != nullptr)
	{
		delete data;
		data = nullptr;
	}
	if (this->image != nullptr)
	{
		this->image->ReleaseData();
		this->image = nullptr;
	}
}