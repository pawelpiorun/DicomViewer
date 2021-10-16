#include "PointsToLineLeastSquaresAlgorithm3D.h"
#include "LaplacianPointsSmoothener.h"

#define LOGFITTINGINFO

void PointsToLineLeastSquaresAlgorithm3D::Compute(bool smoothen)
{
	if (smoothen)
	{
		this->smoothen = smoothen;
		auto smoothener = new LaplacianPointsSmoothener();
		smoothened = smoothener->SmoothenPoints(points, true);
	}

	ComputeLineYZ();
	ComputeLineXZ();

	ComputeLineXY();
	ComputeLineZY();

	ComputeLineYX();
	ComputeLineZX();

	ComputeLine3D();

	if (line3D.empty())
		ComputeLine3D2();

	if (line3D.empty())
		ComputeLine3D3();
}

void PointsToLineLeastSquaresAlgorithm3D::ComputeLineYZ()
{
	auto mean = CalculateMean();
	auto meanY = mean[1];
	auto meanZ = mean[2];
	double nomSum = 0;
	double denomSum = 0;
	if (smoothen)
	{
		double count = smoothened.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((smoothened[i][1] - meanY) * (smoothened[i][2] - meanZ));
			denomSum += std::pow((smoothened[i][1] - meanY), 2);
		}
	}
	else
	{
		double count = points.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((points[i][1] - meanY) * (points[i][2] - meanZ));
			denomSum += std::pow((points[i][1] - meanY), 2);
		}
	}

	if (denomSum != 0)
	{
		auto beta = nomSum / denomSum;
		auto alpha = meanZ - beta * meanY;
		lineYZ = std::vector<double>();
		lineYZ.push_back(beta);
		lineYZ.push_back(alpha);
	}
}

void PointsToLineLeastSquaresAlgorithm3D::ComputeLineXZ()
{
	auto mean = CalculateMean();
	auto meanX = mean[0];
	auto meanZ = mean[2];
	double nomSum = 0;
	double denomSum = 0;

	if (smoothen)
	{
		double count = smoothened.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((smoothened[i][0] - meanX) * (smoothened[i][2] - meanZ));
			denomSum += std::pow((smoothened[i][0] - meanX), 2);
		}
	}
	else
	{
		double count = points.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((points[i][0] - meanX) * (points[i][2] - meanZ));
			denomSum += std::pow((points[i][0] - meanX), 2);
		}
	}

	if (denomSum != 0)
	{
		auto beta = nomSum / denomSum;
		auto alpha = meanZ - beta * meanX;
		lineXZ = std::vector<double>();
		lineXZ.push_back(beta);
		lineXZ.push_back(alpha);
	}
}

void PointsToLineLeastSquaresAlgorithm3D::ComputeLine3D()
{
	if (lineXZ.empty() || lineYZ.empty())
	{
#ifdef LOGFITTINGINFO
		std::cout << "WARNING: 3D line fitting: Omitting first method. Fitting failed." << std::endl;
#endif
		return;
	}

	// z = ay + b - fitted with least squares
	// z = cx + d - same
	auto a = lineYZ[0];
	auto b = lineYZ[1];
	auto c = lineXZ[0];
	auto d = lineXZ[1];

	if (c == 0 || a == 0)
	{
		if (c == 0 && a == 0)
		{
			if (lineXY.empty() || b != d)
			{
#ifdef LOGFITTINGINFO
				std::cout << "WARNING: 3D line fitting: Omitting first method. ";
				std::cout << "a: " << a << " b: " << b << " c: " << c << " d: " << d << std::endl;
#endif
			}
			else
			{
#ifdef LOGFITTINGINFO
				std::cout << "WARNING: Horizontal line from first method..." << std::endl;
#endif
				// x = 1t + 0
				// y = et + f
				// z = 0t + b
				auto e = lineXY[0];
				auto f = lineXY[1];			
				line3D.push_back(0);
				line3D.push_back(f);
				line3D.push_back(b);

				line3D.push_back(1);
				line3D.push_back(e);
				line3D.push_back(0);
			}
		}
		else if (c == 0)
		{
			// line is vertical in XZ plane
			line3D.push_back(CalculateMean()[0]);
			line3D.push_back((-1) * b / a);
			line3D.push_back(0);

			line3D.push_back(0);
			line3D.push_back(1 / a);
			line3D.push_back(1);
		}
		else if (a == 0)
		{
			// line is vertical in YZ plane
			line3D.push_back((-1) * d / c);
			line3D.push_back(CalculateMean()[1]);
			line3D.push_back(0);

			line3D.push_back(1 / c);
			line3D.push_back(0);
			line3D.push_back(1);
		}
		else
			std::cout << "ERROR #111" << std::endl;
	}
	else
	{
		// anchor point of 3d line will be [-d/c, -b/a, 0]
		line3D.push_back((-1) * d / c);
		line3D.push_back((-1) * b / a);
		line3D.push_back(0);

		// normal vector of 3d line will be [1/c, 1/a, 1]
		line3D.push_back(1 / c);
		line3D.push_back(1 / a);
		line3D.push_back(1);
	}
}


void PointsToLineLeastSquaresAlgorithm3D::ComputeLineXY()
{
	auto mean = CalculateMean();
	auto meanX = mean[0];
	auto meanY = mean[1];
	double nomSum = 0;
	double denomSum = 0;
	if (smoothen)
	{
		double count = smoothened.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((smoothened[i][0] - meanX) * (smoothened[i][1] - meanY));
			denomSum += std::pow((smoothened[i][0] - meanX), 2);
		}
	}
	else
	{
		double count = points.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((points[i][0] - meanX) * (points[i][1] - meanY));
			denomSum += std::pow((points[i][0] - meanX), 2);
		}
	}

	if (denomSum != 0)
	{
		auto beta = nomSum / denomSum;
		auto alpha = meanY - beta * meanX;
		lineXY = std::vector<double>();
		lineXY.push_back(beta);
		lineXY.push_back(alpha);
	}
}
void PointsToLineLeastSquaresAlgorithm3D::ComputeLineZY()
{
	auto mean = CalculateMean();
	auto meanZ = mean[2];
	auto meanY = mean[1];
	double nomSum = 0;
	double denomSum = 0;

	if (smoothen)
	{
		double count = smoothened.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((smoothened[i][2] - meanZ) * (smoothened[i][1] - meanY));
			denomSum += std::pow((smoothened[i][2] - meanZ), 2);
		}
	}
	else
	{
		double count = points.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((points[i][2] - meanZ) * (points[i][1] - meanY));
			denomSum += std::pow((points[i][2] - meanZ), 2);
		}
	}

	if (denomSum != 0)
	{
		auto beta = nomSum / denomSum;
		auto alpha = meanY - beta * meanZ;
		lineZY = std::vector<double>();
		lineZY.push_back(beta);
		lineZY.push_back(alpha);
	}
}

void PointsToLineLeastSquaresAlgorithm3D::ComputeLine3D2()
{
	if (lineXY.empty() || lineZY.empty())
	{
#ifdef LOGFITTINGINFO
		std::cout << "WARNING: 3D line fitting: Omitting second method. Fitting failed." << std::endl;
#endif
		return;
	}

	// y = ex + f
	// y = gz + h
	auto e = lineXY[0];
	auto f = lineXY[1];
	auto g = lineZY[0];
	auto h = lineZY[1];

	if (e == 0 || g == 0)
	{
		if (e == 0 && g == 0)
		{
			if (lineXZ.empty() || f != h)
			{
#ifdef LOGFITTINGINFO
				std::cout << "WARNING: 3D line fitting: Omitting second method. ";
				std::cout << "e: " << e << " f: " << f << " g: " << g << " h: " << h << std::endl;
#endif
			}
			else
			{
#ifdef LOGFITTINGINFO
				std::cout << "WARNING: Horizontal line from second method..." << std::endl;
#endif
				// x = 1t + 0
				// y = 0t + f
				// z = cx + d
				auto c = lineXZ[0];
				auto d = lineXZ[1];
				line3D.push_back(0);
				line3D.push_back(f);
				line3D.push_back(d);

				line3D.push_back(1);
				line3D.push_back(0);
				line3D.push_back(c);
			}
		}
		else if (g == 0)
		{
			// line is vertical in ZY plane
			line3D.push_back((-1) * f / e);
			line3D.push_back(0);
			line3D.push_back(CalculateMean()[2]);

			line3D.push_back(1 / e);
			line3D.push_back(1);
			line3D.push_back(0);
		}
		else if (e == 0)
		{
			// line is vertical in XY plane
			line3D.push_back(CalculateMean()[0]);
			line3D.push_back(0);
			line3D.push_back((-1) * h / g);

			line3D.push_back(0);
			line3D.push_back(0);
			line3D.push_back(1 / g);
		}
		else
			std::cout << "ERROR #222" << std::endl;
	}
	else
	{
		// anchor point of 3d line will be [-d/c, -b/a, 0]
		line3D.push_back((-1) * f / e);
		line3D.push_back(0);
		line3D.push_back((-1) * h / g);

		// normal vector of 3d line will be [1/c, 1/a, 1]
		line3D.push_back(1 / e);
		line3D.push_back(1);
		line3D.push_back(1 / g);
	}
}

void PointsToLineLeastSquaresAlgorithm3D::ComputeLineYX()
{
	auto mean = CalculateMean();
	auto meanY = mean[1];
	auto meanX = mean[0];
	double nomSum = 0;
	double denomSum = 0;

	if (smoothen)
	{
		double count = smoothened.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((smoothened[i][1] - meanY) * (smoothened[i][0] - meanX));
			denomSum += std::pow((smoothened[i][1] - meanY), 2);
		}
	}
	else
	{
		double count = points.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((points[i][1] - meanY) * (points[i][0] - meanX));
			denomSum += std::pow((points[i][1] - meanY), 2);
		}
	}

	if (denomSum != 0)
	{
		auto beta = nomSum / denomSum;
		auto alpha = meanX - beta * meanY;
		lineYX = std::vector<double>();
		lineYX.push_back(beta);
		lineYX.push_back(alpha);
	}
}
void PointsToLineLeastSquaresAlgorithm3D::ComputeLineZX()
{
	auto mean = CalculateMean();
	auto meanZ = mean[2];
	auto meanX = mean[0];
	double nomSum = 0;
	double denomSum = 0;

	if (smoothen)
	{
		double count = smoothened.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((smoothened[i][2] - meanZ) * (smoothened[i][0] - meanX));
			denomSum += std::pow((smoothened[i][2] - meanZ), 2);
		}
	}
	else
	{
		double count = points.size();
		for (int i = 0; i < count; i++)
		{
			nomSum += ((points[i][2] - meanZ) * (points[i][0] - meanX));
			denomSum += std::pow((points[i][2] - meanZ), 2);
		}
	}

	if (denomSum != 0)
	{
		auto beta = nomSum / denomSum;
		auto alpha = meanX - beta * meanZ;
		lineZX = std::vector<double>();
		lineZX.push_back(beta);
		lineZX.push_back(alpha);
	}
}


void PointsToLineLeastSquaresAlgorithm3D::ComputeLine3D3()
{
	if (lineYX.empty() || lineZX.empty())
	{
#ifdef LOGFITTINGINFO
		std::cout << "ERROR: 3D line fitting: Third method failed. Fitting failed." << std::endl;
#endif
		return;
	}

	// x = iy + j
	// x = kz + l
	auto i = lineYX[0];
	auto j = lineYX[1];
	auto k = lineZX[0];
	auto l = lineZX[1];

	if (i == 0 || k == 0)
	{
		if (i == 0 && k == 0)
		{
			if (lineYZ.empty() || j != l)
			{
#ifdef LOGFITTINGINFO
				std::cout << "ERROR: 3D line fitting. Third method failed. ";
				std::cout << "i: " << i << " j: " << j << " k: " << k << " l: " << l << std::endl;
#endif
			}
			else
			{
#ifdef LOGFITTINGINFO
				std::cout << "WARNING: Horizontal line from second method..." << std::endl;
#endif
				// x = 0t + j
				// y = 1t + 0
				// z = ay + b
				auto a = lineYZ[0];
				auto b = lineYZ[1];
				line3D.push_back(j);
				line3D.push_back(0);
				line3D.push_back(b);

				line3D.push_back(0);
				line3D.push_back(1);
				line3D.push_back(a);
			}
		}
		else if (k == 0)
		{
			// line is vertical in ZY plane
			line3D.push_back(0);
			line3D.push_back((-1) * j / i);
			line3D.push_back(CalculateMean()[2]);

			line3D.push_back(1);
			line3D.push_back(1 / i);
			line3D.push_back(0);
		}
		else if (i == 0)
		{
			// line is vertical in XY plane
			line3D.push_back(0);
			line3D.push_back(CalculateMean()[1]);
			line3D.push_back((-1) * l / k);

			line3D.push_back(1);
			line3D.push_back(0);
			line3D.push_back(1 / k);
		}
		else
			std::cout << "ERROR #333" << std::endl;

	}
	else
	{
		line3D.push_back(0);
		line3D.push_back((-1) * j / i);
		line3D.push_back((-1) * l / k);

		line3D.push_back(1);
		line3D.push_back(1 / i);
		line3D.push_back(1 / k);
	}
}

std::vector<double> PointsToLineLeastSquaresAlgorithm3D::CalculateMean()
{
	double xSum = 0, ySum = 0, zSum = 0;
	double count = smoothen ? smoothened.size() : points.size();
	for (int i = 0; i < count; i++)
	{
		xSum += smoothen ? smoothened[i][0] : points[i][0];
		ySum += smoothen ? smoothened[i][1] : points[i][1];
		zSum += smoothen ? smoothened[i][2] : points[i][2];
	}
	xSum /= count;
	ySum /= count;
	zSum /= count;

	return std::vector<double> { xSum, ySum, zSum };
}