#pragma once
#include "DicomProcessing.h"
#include "DicomImage.h"

class RamusContoursToRamusLineAlgorithm : public itk::ProcessObject
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(RamusContoursToRamusLineAlgorithm);

	using Self = RamusContoursToRamusLineAlgorithm;
	using Superclass = Object;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(RamusContoursToRamusLineAlgorithm);

	using ImageType = DicomImageType;
	using PointerType = typename ImageType::Pointer;
	using PixelType = typename ImageType::PixelType;

	RamusContoursToRamusLineAlgorithm();
	~RamusContoursToRamusLineAlgorithm();

	void SetImage(ImageType::Pointer image);
	void SetForegroundValue(ImageType::PixelType value);
	ImageType::PixelType GetForegroundValue();
	void Compute();
	void SetIterations(int iterations);
	std::vector<double> GetRamusLine(bool withMinError = false);
	bool IsRamusLineValid();

private:
	using NeighborhoodIterator = itk::ConstNeighborhoodIterator<ImageType>;

	std::vector<ImageType::IndexType> GetAllIndexes();
	std::vector<ImageType::IndexType> GetRandomTwoIndexes(std::vector<ImageType::IndexType> indexes);
	double GetError(NeighborhoodIterator nit, ImageType::IndexType first, ImageType::IndexType second, std::vector<double> line);
	double GetGradient(NeighborhoodIterator nit, ImageType::IndexType index);
	std::vector<double> CreateLine(ImageType::IndexType first, ImageType::IndexType second);
	bool IsLineValid(std::vector<double> line);
	double PointToPointDistance(ImageType::IndexType index1, ImageType::IndexType index2);

	int neighbourhoodRadius = 4;
	ImageType::PixelType foregroundValue;
	ImageType::Pointer image;
	int iterations = 25;
	std::vector<double> _bestRamusLine;
	std::vector<std::vector<double>> _bestRamusLines;
	bool isRamusLineValid = false;
};