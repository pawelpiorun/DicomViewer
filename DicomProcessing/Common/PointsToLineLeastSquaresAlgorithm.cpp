#include "PointsToLineLeastSquaresAlgorithm.h"

void PointsToLineLeastSquaresAlgorithm::Compute()
{
	auto mean = CalculateMean();
	auto meanX = mean[0];
	auto meanY = mean[1];
	double count = points.size();
	double nomSum = 0;
	double denomSum = 0;
	for (int i = 0; i < count; i++)
	{
		nomSum += ((points[i][0] - meanX)*(points[i][1] - meanY));
		denomSum += std::pow((points[i][0] - meanX), 2);
	}
	auto beta = nomSum / denomSum;
	auto alpha = meanY - beta * meanX;
	computedLine = std::vector<double>();
	computedLine.push_back(beta);
	computedLine.push_back(alpha);
}
std::vector<double> PointsToLineLeastSquaresAlgorithm::CalculateMean()
{
	double xSum = 0, ySum = 0;
	double count = points.size();
	for (int i = 0; i < count; i++)
	{
		xSum += points[i][0];
		ySum += points[i][1];
	}
	xSum /= count;
	ySum /= count;

	return std::vector<double> { xSum, ySum };
}