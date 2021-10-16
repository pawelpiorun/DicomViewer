#ifndef GRAYIMAGE_HEADER
#define GRAYIMAGE_HEADER

#include "itkCommon.h"

typedef itk::ImageFileReader<GrayImageType> GrayImageReaderType;

class GrayImage
{
public:
	GrayImage();
	void Open(const char* filePath);
	void SetItkImage(itk::SmartPointer<GrayImageType> image);
	BYTE* GetData();
	~GrayImage();
	int GetWidth();
	int GetHeight();

private:
	int width;
	int height;
	unsigned char* data = nullptr;
	itk::SmartPointer<GrayImageType> image;
};

#endif