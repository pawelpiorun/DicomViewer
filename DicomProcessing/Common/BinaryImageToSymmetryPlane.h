#pragma once
#include "DicomSeries.h"
#include "DicomProcessing.h"

// STILL TODO: THIS CLASS IS NOT OPTIMIZED AND VERY SLOW
class BinaryImageToSymmetryPlane : public itk::ProcessObject
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(BinaryImageToSymmetryPlane);

	using Self = BinaryImageToSymmetryPlane;
	using Superclass = Object;
	using Pointer = itk::SmartPointer<Self>;
	using ConstPointer = itk::SmartPointer<const Self>;

	itkNewMacro(BinaryImageToSymmetryPlane);

	using ImageType = DicomSeriesType;
	using PointerType = typename ImageType::Pointer;
	using PixelType = typename ImageType::PixelType;

	void SetImage(ImageType *image);

	void SetForegroundValue(ImageType::PixelType value);

	itkSetMacro(Epsilon, int);
	itkGetMacro(Epsilon, int);

	itkGetMacro(Error, double);

	BinaryImageToSymmetryPlane();
	~BinaryImageToSymmetryPlane() { };

	/* Get plane coefficients, accordingly to equation: Ax + By + Cz + D = 0*/
	std::vector<double> GetCoefficients() { return m_Coefficients; }
	void SetInitialCoefficients(double A, double B, double C, double D);

	// TODO: Still needs optimization, because it's too slow.
	void Compute();

	enum PlaneOrientation
	{
		NotFixed,
		FixedXY,
		FixedXZ,
		FixedYZ
	};

	/* Fix plane orientation to XY, XZ or YZ.
	*  Proper plane equation coefficients will be set to 0.
	*/
	void SetPlaneOrientationType(BinaryImageToSymmetryPlane::PlaneOrientation orientation);
	itkGetEnumMacro(PlaneOrientationType, BinaryImageToSymmetryPlane::PlaneOrientation);


private:
	void ComputeFixedXY();
	void ComputeFixedXZ();
	void ComputeFixedYZ();
	// TODO: Optimize
	double CalculateSymmetryError();
	double CalculateSymmetryErrorXY();
	double CalculateSymmetryErrorXZ();
	double CalculateSymmetryErrorYZ();
	void PrepareBoundsInfo();
	ImageType::RegionType GetBoundedRegion();
	ImageType::IndexType GetReflectedIndex(ImageType::IndexType index);
	double PointToPointDistance(ImageType::IndexType index1, ImageType::IndexType index2);
	bool IsIndexValid(ImageType::IndexType index);
	bool HasForegroundNeighbour(ImageType::IndexType index);
	void PushCoefficients();
	std::vector<double> m_Coefficients;
	double a;
	double b;
	double c;
	double d;
	double denom;
	int m_Epsilon = 0;
	double m_Error = 0;

	ImageType::PixelType m_ForegroundValue;
	ImageType::Pointer m_Image;
	long m_ImageVoxelCount;
	std::vector<std::vector<int>> m_ImageBoundingBoxXYZ;
	PlaneOrientation m_PlaneOrientationType;
};