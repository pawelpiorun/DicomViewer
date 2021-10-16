#include "BinaryImageToSymmetryPlane.h"

BinaryImageToSymmetryPlane::BinaryImageToSymmetryPlane()
{

}

void BinaryImageToSymmetryPlane::Compute()
{
	m_Coefficients.clear();
	m_Error = 0;

	if (m_Image == nullptr) return;

	// TODO - compute not fixed planes, maybe RANSAC?
	if (m_PlaneOrientationType == PlaneOrientation::NotFixed) return;

	PrepareBoundsInfo();

	if (m_PlaneOrientationType == PlaneOrientation::FixedXY)
		ComputeFixedXY();
	else if (m_PlaneOrientationType == PlaneOrientation::FixedXZ)
		ComputeFixedXZ();
	else if (m_PlaneOrientationType == PlaneOrientation::FixedYZ)
		ComputeFixedYZ();

	PushCoefficients();
}

void BinaryImageToSymmetryPlane::PrepareBoundsInfo()
{
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto inputRegion = m_Image->GetLargestPossibleRegion();
	auto iterator = ConstIteratorType(m_Image, inputRegion);
	iterator.GoToBegin();

	long voxelCount = 0;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == m_ForegroundValue)
			voxelCount++;
		++iterator;
	}

	m_ImageVoxelCount = voxelCount;
	m_ImageBoundingBoxXYZ = DicomProcessing::GetSelectionBoundingBoxXYZ(m_Image, m_ForegroundValue);

	std::cout << "[BinaryImageToSymmetryPlane] Image voxel count: " << m_ImageVoxelCount << std::endl;
}

void BinaryImageToSymmetryPlane::SetImage(ImageType* image)
{
	m_Image = image;
}

void BinaryImageToSymmetryPlane::SetPlaneOrientationType(const BinaryImageToSymmetryPlane::PlaneOrientation orientation)
{
	m_PlaneOrientationType = orientation;
	if (m_PlaneOrientationType == PlaneOrientation::FixedXY)
		SetInitialCoefficients(0, 0, 1, 1);
	else if (m_PlaneOrientationType == PlaneOrientation::FixedXZ)
		SetInitialCoefficients(0, 1, 0, 1);
	else if (m_PlaneOrientationType == PlaneOrientation::FixedYZ)
		SetInitialCoefficients(1, 0, 0, 1);
	else
		SetInitialCoefficients(1, 1, 1, 1);
}

void BinaryImageToSymmetryPlane::SetInitialCoefficients(double A, double B, double C, double D)
{
	a = A; b = B; c = C; d = D;
}

void BinaryImageToSymmetryPlane::ComputeFixedXY()
{
	auto sizeZ = m_ImageBoundingBoxXYZ[1][2]; // max
	d = m_ImageBoundingBoxXYZ[0][2]; // min
	int bestZ = d;
	double minErr = CalculateSymmetryErrorXY();
	for (d = d + 1; d < sizeZ; d++)
	{
		double err = CalculateSymmetryErrorXY();
		if (err < minErr)
		{
			bestZ = d;
			minErr = err;
		}
	}

	d = bestZ;
	m_Error = minErr;
}

void BinaryImageToSymmetryPlane::ComputeFixedXZ()
{
	auto sizeY = m_ImageBoundingBoxXYZ[1][1]; // max
	d = m_ImageBoundingBoxXYZ[0][1]; // min
	int bestY = d;
	double minErr = CalculateSymmetryErrorXZ();
	for (d = d + 1; d < sizeY; d++)
	{
		double err = CalculateSymmetryErrorXZ();
		if (err < minErr)
		{
			bestY = d;
			minErr = err;
		}
	}

	d = bestY;
	m_Error = minErr;
}

void BinaryImageToSymmetryPlane::ComputeFixedYZ()
{
	auto sizeX = m_ImageBoundingBoxXYZ[1][0]; // max
	d = m_ImageBoundingBoxXYZ[0][0]; // min
	int bestX = d;
	double minErr = CalculateSymmetryErrorYZ();
	for (d = d + 1; d < sizeX; d++)
	{
		double err = CalculateSymmetryErrorYZ();
		if (err < minErr)
		{
			bestX = d;
			minErr = err;
		}
	}

	d = bestX;
	m_Error = minErr;
}

double BinaryImageToSymmetryPlane::CalculateSymmetryErrorXY()
{
	denom = a * a + b * b + c * c;

	// go through the bounding box
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto inputRegion = GetBoundedRegion();
	auto iterator = ConstIteratorType(m_Image, inputRegion);
	iterator.GoToBegin();

	double err = 0;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == m_ForegroundValue)
		{
			auto index = iterator.GetIndex();
			auto reflectedIndex = GetReflectedIndex(index);
			if (IsIndexValid(reflectedIndex))
			{
				if (!HasForegroundNeighbour(reflectedIndex))
					err++;
			}
			else
				err++;
		}
		++iterator;
	}
	return err;
}
double BinaryImageToSymmetryPlane::CalculateSymmetryErrorXZ()
{
	denom = a * a + b * b + c * c;

	// we have to go through all of the image
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto inputRegion = GetBoundedRegion();
	auto iterator = ConstIteratorType(m_Image, inputRegion);
	iterator.GoToBegin();

	double err = 0;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == m_ForegroundValue)
		{
			auto index = iterator.GetIndex();
			auto reflectedIndex = GetReflectedIndex(index);
			if (IsIndexValid(reflectedIndex))
			{
				if (!HasForegroundNeighbour(reflectedIndex))
					err++;
			}
			else
				err++;
		}
		++iterator;
	}
	return err;
}
double BinaryImageToSymmetryPlane::CalculateSymmetryErrorYZ()
{
	denom = a * a + b * b + c * c;

	// we have to go through all of the image
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto inputRegion = GetBoundedRegion();
	auto iterator = ConstIteratorType(m_Image, inputRegion);
	iterator.GoToBegin();

	double err = 0;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == m_ForegroundValue)
		{
			auto index = iterator.GetIndex();
			auto reflectedIndex = GetReflectedIndex(index);
			if (IsIndexValid(reflectedIndex))
			{
				if (!HasForegroundNeighbour(reflectedIndex))
					err++;
			}
			else
				err++;
		}
		++iterator;
	}
	return err;
}
double BinaryImageToSymmetryPlane::CalculateSymmetryError()
{
	denom = a * a + b * b + c * c;

	// we have to go through all of the image
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto inputRegion = m_Image->GetLargestPossibleRegion();
	auto iterator = ConstIteratorType(m_Image, inputRegion);
	iterator.GoToBegin();

	double err = 0;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == m_ForegroundValue)
		{
			auto index = iterator.GetIndex();
			auto reflectedIndex = GetReflectedIndex(index);
			if (IsIndexValid(reflectedIndex))
			{
				if (!HasForegroundNeighbour(reflectedIndex))
					err++;
			}
			else
				err++;
		}
		++iterator;
	}
	return err;
}


BinaryImageToSymmetryPlane::ImageType::RegionType
BinaryImageToSymmetryPlane::GetBoundedRegion()
{
	ImageType::RegionType inputRegion;

	ImageType::RegionType::IndexType inputStart;
	ImageType::RegionType::SizeType  size;

	inputStart[0] = m_ImageBoundingBoxXYZ[0][0];
	inputStart[1] = m_ImageBoundingBoxXYZ[0][1];
	inputStart[2] = m_ImageBoundingBoxXYZ[0][2];

	size[0] = m_ImageBoundingBoxXYZ[1][0] - inputStart[0];
	size[1] = m_ImageBoundingBoxXYZ[1][1] - inputStart[1];
	size[2] = m_ImageBoundingBoxXYZ[1][2] - inputStart[2];

	inputRegion.SetSize(size);
	inputRegion.SetIndex(inputStart);

	return inputRegion;
}

BinaryImageToSymmetryPlane::ImageType::IndexType
BinaryImageToSymmetryPlane::GetReflectedIndex(ImageType::IndexType index)
{
	auto x0 = &index[0];
	auto y0 = &index[1];
	auto z0 = &index[2];
	if (denom > (-1) * 0.000001 && denom < 0.000001)
	{
		std::cout << "Cannot reflect point through plane: P(" << *x0 << "," << *y0 << "," << *z0 << ") \t " << a << "x + " << b << "y + " << c << "z - " << d << " = 0" << std::endl;
		return index;
	}

	auto nom = (-1) * ((-1)*d + a * (*x0) + b * (*y0) + c * (*z0));
	auto t = nom / denom;
	auto vector = new double[3] { (*x0) + t * a - (*x0), (*y0) + t * b - (*y0), (*z0) + t * c - (*z0) };
	auto reflectedIndex = ImageType::IndexType();
	reflectedIndex[0] = (*x0) + 2 * vector[0];
	reflectedIndex[1] = (*y0) + 2 * vector[1];
	reflectedIndex[2] = (*z0) + 2 * vector[2];

	std::cout << "Reflected point through plane: P(" << *x0 << "," << *y0 << "," << *z0 << ") \t " << a << "x + " << b << "y + " << c << "z - " << d << " = 0";
	std::cout << " Q(" << reflectedIndex[0] << "," << reflectedIndex[1] << "," << reflectedIndex[2] << ")" << std::endl;

	return reflectedIndex;
}

double BinaryImageToSymmetryPlane::PointToPointDistance(ImageType::IndexType index1, ImageType::IndexType index2)
{
	return std::sqrt(std::pow(index1[0] - index2[0], 2) + std::pow(index1[1] - index2[1], 2) + std::pow(index1[2] - index2[2], 2));
}

bool BinaryImageToSymmetryPlane::IsIndexValid(ImageType::IndexType index)
{
	auto size = m_Image->GetLargestPossibleRegion().GetSize();
	if (index[0] > size[0] || index[0] < size[0]) return false;
	if (index[1] > size[1] || index[1] < size[1]) return false;
	if (index[2] > size[2] || index[2] < size[2]) return false;

	return true;
}

// todo: use neighbourhood iterators
bool BinaryImageToSymmetryPlane::HasForegroundNeighbour(ImageType::IndexType index)
{
	std::cout << "Checking reflected point: P(" << index[0] << "," << index[1] << "," << index[2] << ")" << std::endl;
	if (m_Image->GetPixel(index) == m_ForegroundValue) return true;

	for (int i = (-1) * m_Epsilon; i <= m_Epsilon; i++)
	{
		for (int j = (-1) * m_Epsilon; j <= m_Epsilon; j++)
		{
			for (int k = (-1) * m_Epsilon; k <= m_Epsilon; k++)
			{
				ImageType::IndexType diff = { i, j, k };
				ImageType::IndexType nIndex = { index[0] - i, index[1] - j, index[2] - k };
				if (!IsIndexValid(nIndex)) continue;

				if (m_Image->GetPixel(nIndex) == m_ForegroundValue) return true;
			}
		}
	}

	return false;
}

void BinaryImageToSymmetryPlane::SetForegroundValue(ImageType::PixelType value)
{
	m_ForegroundValue = value;
}

void BinaryImageToSymmetryPlane::PushCoefficients()
{
	if (m_Coefficients.size() == 0)
		m_Coefficients = std::vector<double>(4);

	m_Coefficients.push_back(a);
	m_Coefficients.push_back(b);
	m_Coefficients.push_back(c);
	m_Coefficients.push_back(d);
}