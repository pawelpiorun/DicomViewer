#pragma once
#include "DicomProcessing.h"
#include "DicomImage.h"

class PointsToLineLeastSquaresAlgorithm : public itk::ProcessObject
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(PointsToLineLeastSquaresAlgorithm);

	using Self = PointsToLineLeastSquaresAlgorithm;
	using Superclass = Object;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(PointsToLineLeastSquaresAlgorithm);

	PointsToLineLeastSquaresAlgorithm() { };
	~PointsToLineLeastSquaresAlgorithm() { };
	using ImageType = DicomImageType;
	using PointerType = typename ImageType::Pointer;
	using PixelType = typename ImageType::PixelType;

	void Compute();
	std::vector<double> GetLine() { return this->computedLine; }
	double GetError() { return this->error; }
	void SetPoints(std::vector<ImageType::IndexType> points) { this->points = points; }

private:
	std::vector<double> CalculateMean();
	std::vector<ImageType::IndexType> points;
	std::vector<double> computedLine;
	double error;
};