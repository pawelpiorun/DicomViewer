#pragma once
#include "DicomProcessing.h"
#include "DicomImage.h"
#include "PointsToLineLeastSquaresAlgorithm.h"

class RamusContoursToNotchLineAlgorithm : public itk::ProcessObject
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(RamusContoursToNotchLineAlgorithm);

	using Self = RamusContoursToNotchLineAlgorithm;
	using Superclass = Object;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(RamusContoursToNotchLineAlgorithm);

	using ImageType = DicomImageType;
	using PointerType = typename ImageType::Pointer;
	using PixelType = typename ImageType::PixelType;

	RamusContoursToNotchLineAlgorithm() { };
	~RamusContoursToNotchLineAlgorithm();

	void SetImage(ImageType::Pointer image);
	void SetRamusLine(std::vector<double> ramusLine);
	void SetForegroundValue(ImageType::PixelType value);
	ImageType::PixelType GetForegroundValue();
	void Compute();
	std::vector<double> GetNotchLine();
	ImageType::IndexType GetNotchPoint();
	std::vector<double> GetNotchRamusLine();

private:
	using NeighborhoodIterator = itk::ConstNeighborhoodIterator<ImageType>;
	std::vector<double> CreatePerpendicularLine(ImageType::IndexType index, double slope);
	bool AreAllNeighborsAbove(NeighborhoodIterator nit, ImageType::IndexType index, std::vector<double> line);
	bool AreAllNeighborsBilateral(NeighborhoodIterator nit, ImageType::IndexType index, std::vector<double> line);
	double GetError(NeighborhoodIterator nit, ImageType::IndexType index, std::vector<double> line);
	double GetGradient(NeighborhoodIterator nit, ImageType::IndexType index);

	int neighbourhoodRadius = 10;
	int minNeighborsCount = 5;
	ImageType::PixelType foregroundValue;
	ImageType::Pointer image;

	std::vector<double> _ramusLine = std::vector<double>();
	std::vector<double> _bestNotchLine = std::vector<double>();
	std::vector<double> _bestNotchRamusLine = std::vector<double>();
	ImageType::IndexType _bestNotchPoint;
};