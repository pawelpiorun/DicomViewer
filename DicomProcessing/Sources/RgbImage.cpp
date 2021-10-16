#include "RgbImage.h"

RgbImage::RgbImage()
{

}

void RgbImage::Open(const char* imagePath)
{
	const auto path = itk::SimpleDataObjectDecorator<std::string>::New();
	path->Set(imagePath);

	auto reader = RgbImageReaderType::New();
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

	this->data = new unsigned char[imageSize];

	auto ptr = image->GetBufferPointer();
	for (int i = 0; i < imageSize; i = i + 3)
	{
		data[i] = ptr->GetBlue();
		data[i + 1] = ptr->GetGreen();
		data[i + 2] = ptr->GetRed();
		++ptr;
	}
}


void RgbImage::Save(const char* imagePath)
{
	using WriterType = itk::ImageFileWriter<RgbImageType>;
	auto writer = WriterType::New();
	writer->SetFileName(imagePath);
	try
	{
		writer->SetInput(this->image);
		writer->Update();
	}
	catch (itk::ExceptionObject err)
	{
		std::cout << err.GetDescription() << std::endl;
	}


	this->filePath = imagePath;
	auto lastSlash = this->filePath.find_last_of('\\');
	auto lastDot = this->filePath.find_last_of('.');
	this->fileName = this->filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
}

void RgbImage::SetItkImage(itk::SmartPointer<RgbImageType> image)
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

void RgbImage::Swap()
{
	auto sourceImage = this->image;
	auto sourceRegion = sourceImage->GetLargestPossibleRegion();
	auto sourceSize = sourceRegion.GetSize();

	auto swapped = RgbImageType::New();
	swapped->SetRegions(sourceRegion);
	swapped->Allocate();
	swapped->SetSpacing(sourceImage->GetSpacing());

	using ConstIteratorType = itk::ImageRegionConstIterator<RgbImageType>;
	using IteratorType = itk::ImageRegionIterator<RgbImageType>;
	auto region = swapped->GetLargestPossibleRegion();
	auto iterator = IteratorType(swapped, region);
	auto sourceIt = ConstIteratorType(sourceImage, sourceRegion);
	iterator.GoToBegin();
	sourceIt.GoToBegin();

	auto width = sourceSize[0];
	auto height = sourceSize[1];
	while (!iterator.IsAtEnd())
	{
		auto val = sourceIt.Value();
		auto in = sourceIt.GetIndex();
		RgbImageType::IndexType swappedIndex = { width - in[0] - 1, height - in[1] - 1 };
		swapped->SetPixel(swappedIndex, val);
		
		++iterator;
		++sourceIt;
	}

	this->image = swapped;
}

BYTE* RgbImage::GetData()
{
	return data;
}

int RgbImage::GetHeight()
{
	return this->height;
}

int RgbImage::GetWidth()
{
	return this->width;
}

RgbImage::~RgbImage()
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