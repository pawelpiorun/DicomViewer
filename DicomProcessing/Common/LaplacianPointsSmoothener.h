#include <vector>
#include "itkCommon.h"

class LaplacianPointsSmoothener
{
public:
	LaplacianPointsSmoothener() { };
	std::vector<std::vector<double>> SmoothenPoints(std::vector<std::vector<double>> points,
		bool reverseOrder = false, int iterations = 100, bool useRelaxation = true, double relaxation = 0.1);

	std::vector<std::vector<double>> SmoothenPoints(std::vector<DicomSeriesType::IndexType> points,
		bool reverseOrder = false, int iterations = 100, bool useRelaxation = true, double relaxation = 0.1);

private:
	void DoSmoothening(int iterations = 100, bool useRelaxation = true, double relaxation = 0.1);
	void DoSmootheningReverse(int iterations = 100, bool useRelaxation = true, double relaxation = 0.1);
	std::vector<std::vector<double>> smoothened;
};
