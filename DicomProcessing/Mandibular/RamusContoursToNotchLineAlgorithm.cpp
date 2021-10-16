#include "RamusContoursToNotchLineAlgorithm.h"

//#define LOGALLNOTCHLINEINFO

void RamusContoursToNotchLineAlgorithm::Compute()
{
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto it = ConstIteratorType(image, image->GetLargestPossibleRegion());
	it.GoToBegin();
	std::vector<double> bestLine;
	ImageType::IndexType bestNotchPoint;
	double minErr = DBL_MAX;
	while (!it.IsAtEnd())
	{
		if (it.Value() == foregroundValue)
		{
			NeighborhoodIterator::RadiusType radius;
			for (unsigned int i = 0; i < ImageType::ImageDimension; ++i) radius[i] = neighbourhoodRadius;
			auto nit = NeighborhoodIterator(radius, image, image->GetLargestPossibleRegion());

			auto index = it.GetIndex();
			auto line = CreatePerpendicularLine(index, _ramusLine[0]);
#ifdef LOGALLNOTCHLINEINFO
			std::cout << "[Notch line] y = " << line[0] << "x + " << line[1] << "\t";
#endif // LOGALLNOTCHLINEINFO

			auto isValid = AreAllNeighborsAbove(nit, index, line) && AreAllNeighborsBilateral(nit, index, line);
			if (isValid)
			{

#ifdef LOGALLNOTCHLINEINFO
				std::cout << "Result: valid" << "\t";
#endif // LOGALLNOTCHLINEINFO

				auto err = GetError(nit, index, line);

#ifdef LOGALLNOTCHLINEINFO
				std::cout << "Error: " << err << std::endl;
#endif // LOGALLNOTCHLINEINFO

				if (err < minErr && !isnan<double>(err))
				{
					minErr = err;
					bestLine = line;
					bestNotchPoint = index;
				}
			}
#ifdef LOGALLNOTCHLINEINFO
			else
				std::cout << "Result: invalid" << std::endl;
#endif // LOGALLNOTCHLINEINFO

		}
		++it;
	}

	if (bestLine.empty() || bestNotchPoint.empty())
	{
		std::cout << "[Notch line] Failed. No valid line or point." << std::endl;
		return;
	}
	std::cout << "[Notch line] Best line: y = " << bestLine[0] << "x + " << bestLine[1] << std::endl;
	std::cout << "[Notch line] Best point: (" << bestNotchPoint[0] << "," << bestNotchPoint[1] << ")" << std::endl;
	this->_bestNotchLine = bestLine;
	this->_bestNotchPoint = bestNotchPoint;
	this->_bestNotchRamusLine = CreatePerpendicularLine(bestNotchPoint, bestLine[0]);
}

bool RamusContoursToNotchLineAlgorithm::AreAllNeighborsAbove(
	NeighborhoodIterator nit, ImageType::IndexType index, std::vector<double> line)
{
	nit.SetLocation(index);
	int count = 0;
	for (unsigned int i = 0; i < nit.Size(); i++)
	{
		if (nit.GetPixel(i) != foregroundValue) continue;
		count++;
		auto in = nit.GetIndex(i);
		auto val = line[0] * in[0] + line[1];
		if (in[1] > val + 2) return false;
	}
	if (count < minNeighborsCount) return false;

	return true;
}
bool RamusContoursToNotchLineAlgorithm::AreAllNeighborsBilateral(NeighborhoodIterator nit,
	ImageType::IndexType index, std::vector<double> line)
{
	nit.SetLocation(index);
	auto symmetryLine = CreatePerpendicularLine(index, line[0]);

	auto lowerCount = 0;
	auto upperCount = 0;
	for (unsigned int i = 0; i < nit.Size(); i++)
	{
		if (nit.GetPixel(i) != foregroundValue) continue;
		auto in = nit.GetIndex(i);
		auto val = symmetryLine[0] * in[0] + symmetryLine[1];
		if (val > in[1]) upperCount++;
		if (val < in[1]) lowerCount++;
	}

	auto tolerance = neighbourhoodRadius / 3;
	if (std::abs(lowerCount - upperCount) < tolerance) return true;

	return false;
}
std::vector<double> RamusContoursToNotchLineAlgorithm::CreatePerpendicularLine(ImageType::IndexType index,
	double sourceSlope)
{
	auto line = std::vector<double>();
	auto slope = - 1 / sourceSlope;
	auto component = index[1] - slope * index[0];
	line.push_back(slope);
	line.push_back(component);
	return std::vector<double> { slope, component };
}

double RamusContoursToNotchLineAlgorithm::GetError(NeighborhoodIterator nit, ImageType::IndexType index, std::vector<double> line)
{
	auto slope = GetGradient(nit, index);
	return std::abs(slope - line[0]);
}

double RamusContoursToNotchLineAlgorithm::GetGradient(NeighborhoodIterator nit, ImageType::IndexType index)
{
	nit.SetLocation(index);
	std::vector<ImageType::IndexType> points = std::vector<ImageType::IndexType>();
	for (unsigned int i = 0; i < nit.Size() - 1; ++i)
	{
		if (nit.GetPixel(i) == foregroundValue)
		{
			points.push_back(nit.GetIndex(i));
		}
	}

	using LinearRegressionType = PointsToLineLeastSquaresAlgorithm;
	auto linearRegression = LinearRegressionType::New();
	linearRegression->SetPoints(points);
	linearRegression->Compute();
	auto line = linearRegression->GetLine();

#ifdef LOGALLNOTCHLINEINFO	
	std::cout << "Local slope: " << line[0] << "\t";
#endif // LOGALLNOTCHLINEINFO

	return line[0];
}


void RamusContoursToNotchLineAlgorithm::SetImage(ImageType::Pointer image)
{
	this->image = image;
}
void RamusContoursToNotchLineAlgorithm::SetRamusLine(std::vector<double> ramusLine)
{
	this->_ramusLine = ramusLine;
}
void RamusContoursToNotchLineAlgorithm::SetForegroundValue(ImageType::PixelType value)
{
	this->foregroundValue = value;
}
RamusContoursToNotchLineAlgorithm::ImageType::PixelType RamusContoursToNotchLineAlgorithm::GetForegroundValue()
{
	return this->foregroundValue;
}
std::vector<double> RamusContoursToNotchLineAlgorithm::GetNotchLine()
{
	return this->_bestNotchLine;
}
RamusContoursToNotchLineAlgorithm::ImageType::IndexType
RamusContoursToNotchLineAlgorithm::GetNotchPoint()
{
	return this->_bestNotchPoint;
}
std::vector<double> RamusContoursToNotchLineAlgorithm::GetNotchRamusLine()
{
	return this->_bestNotchRamusLine;
}

RamusContoursToNotchLineAlgorithm::~RamusContoursToNotchLineAlgorithm()
{
	std::cout << "NotchLineExtractor dctor... ";

	std::cout << "done." << std::endl;
}