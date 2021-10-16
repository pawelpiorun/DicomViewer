#pragma once
#include "DicomProcessing.h"
#include "DicomSeries.h"

class PointsToLineLeastSquaresAlgorithm3D : public itk::ProcessObject
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(PointsToLineLeastSquaresAlgorithm3D);

	using Self = PointsToLineLeastSquaresAlgorithm3D;
	using Superclass = Object;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(PointsToLineLeastSquaresAlgorithm3D);

	PointsToLineLeastSquaresAlgorithm3D() { };
	~PointsToLineLeastSquaresAlgorithm3D() { };
	using ImageType = DicomSeriesType;
	using PointerType = typename ImageType::Pointer;
	using PixelType = typename ImageType::PixelType;

	void Compute(bool smoothen = false);
	std::vector<double> GetLine() { return this->line3D; }
	double GetError() { return this->error; }
	void SetPoints(std::vector<ImageType::IndexType> points) { this->points = points; }

private:
	void ComputeLineYZ();
	void ComputeLineXZ();
	void ComputeLineXY();
	void ComputeLineZY();
	void ComputeLineYX();
	void ComputeLineZX();
	void ComputeLine3D();
	void ComputeLine3D2();
	void ComputeLine3D3();
	std::vector<double> CalculateMean();
	std::vector<ImageType::IndexType> points;
	std::vector<std::vector<double>> smoothened;
	std::vector<double> lineYZ;
	std::vector<double> lineXZ;
	std::vector<double> line3D;

	// second method
	std::vector<double> lineXY;
	std::vector<double> lineZY;

	// third method
	std::vector<double> lineYX;
	std::vector<double> lineZX;

	double error;
	bool smoothen = false;
};