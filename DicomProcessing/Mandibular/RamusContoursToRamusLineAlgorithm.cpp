#include "RamusContoursToRamusLineAlgorithm.h"
#include "PointsToLineLeastSquaresAlgorithm.h"
#include <cstdlib>
#include <ctime>

//#define LOGALLRAMUSLINEINFO

RamusContoursToRamusLineAlgorithm::RamusContoursToRamusLineAlgorithm()
{
	_bestRamusLines = std::vector<std::vector<double>>();
	_bestRamusLine = std::vector<double>();
	_bestRamusLine.push_back(0);
	_bestRamusLine.push_back(0);
	srand(time(0));
}

void RamusContoursToRamusLineAlgorithm::Compute()
{
	NeighborhoodIterator::RadiusType radius;
	for (unsigned int i = 0; i < ImageType::ImageDimension; ++i) radius[i] = neighbourhoodRadius;
	auto nit = NeighborhoodIterator(radius, image, image->GetLargestPossibleRegion());

	auto allIndexes = GetAllIndexes();
	using LinearRegressionType = PointsToLineLeastSquaresAlgorithm;
	auto linearRegression = LinearRegressionType::New();
	linearRegression->SetPoints(allIndexes);
	linearRegression->Compute();
	auto mediumLine = linearRegression->GetLine();
#ifdef LOGALLRAMUSLINEINFO
	std::cout << std::endl << "[Ramus line] Medium line: y = " << mediumLine[0] << "x + " << mediumLine[1] << std::endl;
#endif // LOGALLRAMUSLINEINFO


	std::vector<double> bestLine = std::vector<double>();
	double minErr = DBL_MAX;
	for (int i = 0; i < iterations; i++)
	{
		std::cout << std::fixed;
		std::cout << std::setprecision(2);
 		auto randoms = GetRandomTwoIndexes(allIndexes);
		if (randoms.size() != 2)
			std::cout << std::endl << "[Ramus line] Random indexes count is not 2!" << std::endl;

		auto ramusLine = CreateLine(randoms[0], randoms[1]);
		auto isValid = IsLineValid(ramusLine);
		if (!isValid)
		{
			i--;
			continue;
		}
		auto err = GetError(nit, randoms[0], randoms[1], ramusLine);

#ifdef LOGALLRAMUSLINEINFO
		std::cout << "[Ramus line] It: " << i + 1 << "\t";
		std::cout << " y = " << ramusLine[0] << "x + " << ramusLine[1] << "\t";
		std::cout << "Err: " << err << "\t";
#endif // LOGALLRAMUSLINEINFO


		if (err < minErr && !isnan<double>(err))
		{
			minErr = err;
			bestLine = ramusLine;
		}

		_bestRamusLines.push_back(ramusLine);


#ifdef LOGALLRAMUSLINEINFO
		std::cout << "P1: (" << randoms[0][0] << "," << randoms[0][1] << ")  P2: (" << randoms[1][0] << "," << randoms[1][1] << ")" << std::endl;
#endif // LOGALLRAMUSLINEINFO

	}

	this->_bestRamusLine = bestLine;
	isRamusLineValid = true;
	std::cout << "[Ramus line] Best ramus line: y = " << bestLine[0] << "x + " << bestLine[1] << "\t Error: " << minErr << std::endl;
}


double RamusContoursToRamusLineAlgorithm::GetError(NeighborhoodIterator nit, ImageType::IndexType first, ImageType::IndexType second,
	std::vector<double> line)
{
	auto slope1 = GetGradient(nit, first);
	auto slope2 = GetGradient(nit, second);
	return (std::abs(slope2 - line[0]) + std::abs(slope1 - line[0])) / 2 + std::abs(slope2 - slope1);
}
bool RamusContoursToRamusLineAlgorithm::IsLineValid(std::vector<double> line)
{
	if (line[0] == 0 && line[1] == 0) return false;

	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto it = ConstIteratorType(image, image->GetLargestPossibleRegion());
	it.GoToBegin();
	while (!it.IsAtEnd())
	{
		if (it.Value() == foregroundValue)
		{
			auto in = it.GetIndex();
			auto val = line[0] * in[0] + line[1];
			if (in[1] > val) return false;
		}
		++it;
	}

	return true;
}
std::vector<double> RamusContoursToRamusLineAlgorithm::CreateLine(ImageType::IndexType first, ImageType::IndexType second)
{
	std::vector<double> line = { 0, 0 };
	if (first[0] == second[0]) return line;

	line[0] = (double)(second[1] - first[1]) / (second[0] - first[0]);
	line[1]  = second[1] - (double)(line[0] * second[0]);
	return line;
}

double RamusContoursToRamusLineAlgorithm::GetGradient(NeighborhoodIterator nit, ImageType::IndexType index)
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

#ifdef LOGALLRAMUSLINEINFO
	std::cout << "Local slope: " << line[0] << "\t";
#endif // LOGALLRAMUSLINEINFO

	return line[0];
}

std::vector<RamusContoursToRamusLineAlgorithm::ImageType::IndexType>
RamusContoursToRamusLineAlgorithm::GetAllIndexes()
{
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto it = ConstIteratorType(image, image->GetLargestPossibleRegion());
	it.GoToBegin();
	std::vector<ImageType::IndexType> indexes = std::vector<ImageType::IndexType>();
	while (!it.IsAtEnd())
	{
		if (it.Value() == foregroundValue)
			indexes.push_back(it.GetIndex());
		++it;
	}

	return indexes;
}

std::vector<RamusContoursToRamusLineAlgorithm::ImageType::IndexType>
RamusContoursToRamusLineAlgorithm::GetRandomTwoIndexes(std::vector<ImageType::IndexType> indexes)
{
	std::vector<ImageType::IndexType> randoms = std::vector<ImageType::IndexType>();

	auto size = indexes.size();
	bool isOk = false;
	while (!isOk)
	{
		auto first = rand() % size;
		auto second = first;
		while (second == first)
			second = rand() % size;

		auto in1 = indexes[first];
		auto in2 = indexes[second];
		isOk = PointToPointDistance(in1, in2) > 40;
		if (isOk)
		{
			randoms.push_back(in1);
			randoms.push_back(in2);
		}
	}

	return randoms;
}
double RamusContoursToRamusLineAlgorithm::PointToPointDistance(ImageType::IndexType index1, ImageType::IndexType index2)
{
	return std::sqrt(std::pow(index1[0] - index2[0], 2) + std::pow(index1[1] - index2[1], 2) + std::pow(index1[2] - index2[2], 2));
}
void RamusContoursToRamusLineAlgorithm::SetImage(ImageType::Pointer image)
{
	this->image = image;
}
void RamusContoursToRamusLineAlgorithm::SetIterations(int iterations)
{
	this->iterations = iterations;
}
std::vector<double> RamusContoursToRamusLineAlgorithm::GetRamusLine(bool withMinError)
{
	if (withMinError)
		return this->_bestRamusLine;

	std::vector<double> withMaxSlope = _bestRamusLine;
	for (int i = 0; i < _bestRamusLines.size(); i++)
	{
		if (_bestRamusLines[i][0] > withMaxSlope[0])
			withMaxSlope = _bestRamusLines[i];

		if (_bestRamusLines[i][0] < 0
			&& withMaxSlope[0] > 0)
			withMaxSlope = _bestRamusLines[i];
		else if (_bestRamusLines[i][0] < 0
			&& withMaxSlope[0] < 0
			&& _bestRamusLines[i][0] < withMaxSlope[0])
			withMaxSlope = _bestRamusLines[i];
	}

	return withMaxSlope;
}
bool RamusContoursToRamusLineAlgorithm::IsRamusLineValid()
{
	return this->isRamusLineValid;
}
void RamusContoursToRamusLineAlgorithm::SetForegroundValue(ImageType::PixelType value)
{
	this->foregroundValue = value;
}
RamusContoursToRamusLineAlgorithm::ImageType::PixelType RamusContoursToRamusLineAlgorithm::GetForegroundValue()
{
	return this->foregroundValue;
}

RamusContoursToRamusLineAlgorithm::~RamusContoursToRamusLineAlgorithm()
{
	std::cout << "RamusLineExtractor dctor... ";

	std::cout << "done." << std::endl;
}