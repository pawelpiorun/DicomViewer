#ifndef RGBIMAGE_HEADER
#define RGBIMAGE_HEADER

#include "itkCommon.h"

typedef itk::ImageFileReader<RgbImageType> RgbImageReaderType;

class RgbImage
{
	public:
		RgbImage();
		void Open(const char* filePath);
		void Save(const char* filePath);
		void SetItkImage(itk::SmartPointer<RgbImageType> image);
		BYTE* GetData();
		~RgbImage();
		int GetWidth();
		int GetHeight();
		std::string GetFileName() { return this->fileName; }

		void Swap();
		void SetImage(RgbImageType::Pointer newImage) { this->image = newImage; }
		RgbImageType::Pointer GetImage() { return this->image; }

	private:
		int width;
		int height;
		unsigned char* data = nullptr;
		itk::SmartPointer<RgbImageType> image;
		std::string fileName;
		std::string filePath;
};

#endif