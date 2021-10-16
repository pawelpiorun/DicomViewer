#include "LaplacianPointsSmoothener.h"

std::vector<std::vector<double>> 
LaplacianPointsSmoothener::SmoothenPoints(std::vector<std::vector<double>> points,
	bool reverseOrder, int iterations, bool useRelaxation, double relaxation)
{
	smoothened = std::vector<std::vector<double>>();
	auto count = points.size();
	for (int i = 0; i < count; i++)
		smoothened.push_back(std::vector<double> {points[i][0], points[i][1], points[i][2]});

	if (reverseOrder)
		DoSmootheningReverse(iterations, useRelaxation, relaxation);
	else
		DoSmoothening(iterations, useRelaxation, relaxation);

	return smoothened;
}


std::vector<std::vector<double>>
LaplacianPointsSmoothener::SmoothenPoints(std::vector<DicomSeriesType::IndexType> points,
	bool reverseOrder, int iterations, bool useRelaxation, double relaxation)
{
	smoothened = std::vector < std::vector<double>>();
	auto count = points.size();
	for (int i = 0; i < count; i++)
		smoothened.push_back(std::vector<double> { (double)points[i][0], (double)points[i][1], (double)points[i][2]});

	if (reverseOrder)
		DoSmootheningReverse(iterations, useRelaxation, relaxation);
	else
		DoSmoothening(iterations, useRelaxation, relaxation);

	return smoothened;
}


void LaplacianPointsSmoothener::DoSmoothening(int iterations, bool useRelaxation, double relaxation)
{
	for (int i = 0; i < iterations; i++)
	{
		for (int j = 1; j < smoothened.size() - 1; j++)
		{
			auto prev = smoothened[j - 1];
			auto curr = smoothened[j];
			auto next = smoothened[j + 1];

			if (useRelaxation)
			{
				curr[0] += relaxation * (0.5 * (prev[0] + next[0]) - curr[0]);
				curr[1] += relaxation * (0.5 * (prev[1] + next[1]) - curr[1]);
				curr[2] += relaxation * (0.5 * (prev[2] + next[2]) - curr[2]);
			}
			else
			{
				curr[0] = 0.5 * (prev[0] + next[0]);
				curr[1] = 0.5 * (prev[1] + next[1]);
				curr[2] = 0.5 * (prev[2] + next[2]);
			}

			smoothened[j] = curr;
		}
	}
}
void LaplacianPointsSmoothener::DoSmootheningReverse(
	int iterations, bool useRelaxation, double relaxation)
{
	for (int i = 0; i < iterations; i++)
	{
		for (int j = smoothened.size() - 2; j > 0; j--)
		{
			auto prev = smoothened[j + 1];
			auto curr = smoothened[j];
			auto next = smoothened[j - 1];

			if (useRelaxation)
			{
				curr[0] += relaxation * (0.5 * (prev[0] + next[0]) - curr[0]);
				curr[1] += relaxation * (0.5 * (prev[1] + next[1]) - curr[1]);
				curr[2] += relaxation * (0.5 * (prev[2] + next[2]) - curr[2]);
			}
			else
			{
				curr[0] = 0.5 * (prev[0] + next[0]);
				curr[1] = 0.5 * (prev[1] + next[1]);
				curr[2] = 0.5 * (prev[2] + next[2]);
			}

			smoothened[j] = curr;
		}
	}
}