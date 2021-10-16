#include "DicomProcessing.h"
#include "vnl_svd.h"
#include "ITK-5.1/itkVariableSizeMatrix.h"
#include "vnl_matrix.h"
#include "vnl_determinant.h"

itk::SmartPointer<DicomSeriesType> DicomProcessing::ThresholdBetween(DicomSeries* dicomSeries, short lower, short upper)
{
	using ThresholdType = itk::BinaryThresholdImageFilter<DicomSeriesType, DicomSeriesType>;

	auto threshold = ThresholdType::New();
	threshold->SetInput(dicomSeries->GetItkImageData());

	threshold->SetUpperThreshold(upper);
	threshold->SetLowerThreshold(lower);
	threshold->SetInsideValue(1024);
	threshold->SetOutsideValue(-2048);

	try
	{
		threshold->Update();
	}
	catch (itk::ExceptionObject e)
	{
		std::cout << e.GetDescription() << std::endl;
	}

	return threshold->GetOutput();
}

itk::SmartPointer<DicomSeriesType> DicomProcessing::ThresholdBelow(DicomSeries* dicomSeries, short thresholdValue)
{
	using ThresholdType = itk::BinaryThresholdImageFilter<DicomSeriesType, DicomSeriesType>;

	auto threshold = ThresholdType::New();

	threshold->SetInput(dicomSeries->GetItkImageData());

	threshold->SetUpperThreshold(thresholdValue);
	threshold->SetInsideValue(1024);
	threshold->SetOutsideValue(-2048);

	try
	{
		threshold->Update();
	}
	catch (itk::ExceptionObject e)
	{
		std::cout << e.GetDescription() << std::endl;
	}

	return threshold->GetOutput();
}

itk::SmartPointer<DicomSeriesType> DicomProcessing::ConnectedThreshold(DicomSeries* dicomSeries, short lower, short upper)
{
	using ConnectedThresholdType = itk::ConnectedThresholdImageFilter<DicomSeriesType, DicomSeriesType>;

	auto threshold = ConnectedThresholdType::New();
	threshold->SetInput(dicomSeries->GetItkImageData());

	threshold->SetLower(lower);
	threshold->SetUpper(upper);
	threshold->SetReplaceValue(1024);
	try
	{
		threshold->Update();
	}
	catch (itk::ExceptionObject e)
	{
		std::cout << e.GetDescription() << std::endl;
	}

	return threshold->GetOutput();
}

itk::SmartPointer<DicomSeriesType> DicomProcessing::Dilate(itk::SmartPointer<DicomSeriesType> input,
	int radiusValue, int iterations)
{
	if (iterations == 0) return input;

	using KernelType = itk::BinaryBallStructuringElement<DicomSeriesType::PixelType, DicomSeriesType::ImageDimension>;
	using DilateType = itk::BinaryDilateImageFilter<DicomSeriesType, DicomSeriesType, KernelType>;

	auto dilate = DilateType::New();
	auto kernel = KernelType();

	KernelType::RadiusType radius;
	radius.Fill(radiusValue);
	kernel.SetRadius(radius);
	kernel.CreateStructuringElement();

	dilate->SetForegroundValue(1024);
	dilate->SetKernel(kernel);
	dilate->SetInput(input);

	try
	{
		dilate->Update();
	}
	catch (itk::ExceptionObject x)
	{
		cout << x.GetDescription() << endl;
	}

	return Dilate(dilate->GetOutput(), radiusValue, --iterations);
}

itk::SmartPointer<DicomSeriesType> DicomProcessing::Erode(itk::SmartPointer<DicomSeriesType> input,
	int radiusValue, int iterations)
{
	if (iterations == 0) return input;

	using KernelType = itk::FlatStructuringElement<DicomSeriesType::ImageDimension>;
	using ErodeType = itk::BinaryErodeImageFilter<DicomSeriesType, DicomSeriesType, KernelType>;

	auto erode = ErodeType::New();

	KernelType::RadiusType radius;
	radius.Fill(radiusValue);

	auto kernel = KernelType::Box(radius);

	erode->SetForegroundValue(1024);
	erode->SetKernel(kernel);
	erode->SetInput(input);
	erode->SetNumberOfWorkUnits(6);

	try
	{
		erode->Update();
	}
	catch (itk::ExceptionObject x)
	{
		cout << x.GetDescription() << endl;
	}

	return Erode(erode->GetOutput(), radiusValue, --iterations);
}

itk::SmartPointer<DicomSeriesType> DicomProcessing::GetExternalAir(itk::SmartPointer<DicomSeriesType> image)
{
	using ConnectedThresholdType = itk::ConnectedThresholdImageFilter<DicomSeriesType, DicomSeriesType>;

	auto corners = GetCorners(image);

	auto threshold = ConnectedThresholdType::New();
	for (int i = 0; i < corners.size(); i++)
		threshold->AddSeed(corners[i]);

	threshold->SetInput(image);
	threshold->SetLower(0);
	threshold->SetReplaceValue(1024);
	try
	{
		threshold->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}

	return threshold->GetOutput();
}

std::vector<DicomSeriesType::IndexType> DicomProcessing::GetCorners(itk::SmartPointer<DicomSeriesType> image)
{
	auto region = image->GetLargestPossibleRegion();
	auto corner = image->GetLargestPossibleRegion().GetUpperIndex();
	auto x = corner[0];
	auto y = corner[1];
	auto z = corner[2];

	std::vector<DicomSeriesType::IndexType> corners;
	corners.push_back({ 0, 0, 0 });
	corners.push_back({ 0, 0, z });
	corners.push_back({ 0, y, 0 });
	corners.push_back({ 0, y, z });

	corners.push_back({ x, 0, 0 });
	corners.push_back({ x, 0, z });
	corners.push_back({ x, y, 0 });
	corners.push_back({ x, y, z });

	return corners;
}

itk::SmartPointer<DicomSeriesType> DicomProcessing::Subtract(
	itk::SmartPointer<DicomSeriesType> minuend,
	itk::SmartPointer<DicomSeriesType> subtrahend)
{
	using SubtractFilterType = itk::SubtractImageFilter<DicomSeriesType, DicomSeriesType, DicomSeriesType>;

	auto subtract = SubtractFilterType::New();
	subtract->SetInput1(minuend);
	subtract->SetInput2(subtrahend);

	try
	{
		subtract->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}

	return subtract->GetOutput();
}

itk::SmartPointer<LabeledSeriesType> DicomProcessing::WatershedSegmentation(
	itk::SmartPointer<DicomSeriesType> image,
	short threshold,
	double level)
{
	using WatershedFilterType = itk::WatershedImageFilter<DicomSeriesType>;

	auto segmentation = WatershedFilterType::New();
	segmentation->SetInput(image);
	segmentation->SetThreshold(threshold);
	segmentation->SetLevel(level);

	try
	{
		segmentation->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}

	return segmentation->GetOutput();
}

//void DicomProcessing::WriteStatistics(itk::SmartPointer<DicomSeriesType> image)
//{
//	using StatisticsFilter = itk::StatisticsImageFilter<DicomSeriesType>;
//
//	auto statistics = StatisticsFilter::New();
//	statistics->SetInput(image);
//
//	try
//	{
//		statistics->Update();
//	}
//	catch (itk::ExceptionObject x)
//	{
//		std::cout << x.GetDescription() << std::endl;
//	}
//
//	std::cout << "Min: " << statistics->GetMinimum() << std::endl;
//	std::cout << "Max: " << statistics->GetMaximum() << std::endl;
//	std::cout << "CRN: " << statistics->GetCurrentRequestNumber() << std::endl;
//	std::cout << "Sum: " << statistics->GetSum() << std::endl;
//}

int DicomProcessing::GetVoxelCount(DicomSeriesType* image, DicomSeriesType::PixelType value)
{
	using ConstIteratorType = itk::ImageRegionConstIterator<DicomSeriesType>;

	auto inputRegion = image->GetLargestPossibleRegion();
	auto iterator = ConstIteratorType(image, inputRegion);
	iterator.GoToBegin();

	int count = 0;
	auto points = std::vector<DicomSeriesType::IndexType>();
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == value)
			count++;
		++iterator;
	}
	return count;
}

std::vector<std::vector<int>> DicomProcessing::GetSelectionBoundingBoxXYZ(DicomSeriesType* image, DicomSeriesType::PixelType selectionValue)
{
	using ConstIteratorType = itk::ImageRegionConstIterator<DicomSeriesType>;

	auto inputRegion = image->GetLargestPossibleRegion();
	auto iterator = ConstIteratorType(image, inputRegion);
	iterator.GoToBegin();

	auto size = inputRegion.GetSize();
	std::vector<int> minPoint = std::vector<int>(3);
	minPoint = { (int)size[0], (int)size[1], (int)size[2] };
	std::vector<int> maxPoint = std::vector<int>(3);
	maxPoint = { 0, 0, 0, };
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == selectionValue)
		{
			auto index = iterator.GetIndex();
			if (index[0] < minPoint[0]) minPoint[0] = index[0];
			if (index[1] < minPoint[1]) minPoint[1] = index[1];
			if (index[2] < minPoint[2]) minPoint[2] = index[2];
			if (index[0] > maxPoint[0]) maxPoint[0] = index[0];
			if (index[1] > maxPoint[1]) maxPoint[1] = index[1];
			if (index[2] > maxPoint[2]) maxPoint[2] = index[2];
		}
		++iterator;
	}

	auto points = std::vector<std::vector<int>>();
	points.push_back(minPoint);
	points.push_back(maxPoint);

	std::cout << "[BoundingBoxXYZ] Bounding box: (" << points[0][0] << "," << points[0][1] << "," << points[0][2] << ")";
	std::cout << "- (" << points[1][0] << "," << points[1][1] << "," << points[1][2] << ")\n";

	return points;
}

DicomSeriesType::IndexType
DicomProcessing::GetMediumPoint(DicomSeriesType::Pointer image, DicomSeriesType::PixelType _foregroundValue)
{
	using ConstIteratorType = itk::ImageRegionConstIterator<DicomSeriesType>;
	auto inputRegion = image->GetLargestPossibleRegion().GetSize();
	auto iterator = ConstIteratorType(image, inputRegion);
	iterator.GoToBegin();

	int maxZ = INT_MIN, maxY = INT_MIN, maxX = INT_MIN;
	int minZ = INT_MAX, minY = INT_MAX, minX = INT_MAX;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _foregroundValue)
		{
			if (iterator.GetIndex()[2] > maxZ) maxZ = iterator.GetIndex()[2];
			if (iterator.GetIndex()[1] > maxY) maxY = iterator.GetIndex()[1];
			if (iterator.GetIndex()[0] > maxX) maxX = iterator.GetIndex()[0];

			if (iterator.GetIndex()[2] < minZ) minZ = iterator.GetIndex()[2];
			if (iterator.GetIndex()[1] < minY) minY = iterator.GetIndex()[1];
			if (iterator.GetIndex()[0] < minX) minX = iterator.GetIndex()[0];
		}
		++iterator;
	}

	auto mediumZ = minZ + (maxZ - minZ) / 2;
	auto mediumY = minY + (maxY - minY) / 2;
	auto mediumX = minX + (maxX - minX) / 2;
	std::cout << "[Medium point] X: = " << mediumX << "\t Y = " << mediumY << "\t Z = " << mediumZ << std::endl;
	DicomSeriesType::IndexType point = { mediumX, mediumY, mediumZ };
	return point;
}


DicomImageType::IndexType
DicomProcessing::GetMediumPoint(DicomImageType::Pointer image, DicomImageType::PixelType _foregroundValue)
{
	using ConstIteratorType = itk::ImageRegionConstIterator<DicomImageType>;
	auto inputRegion = image->GetLargestPossibleRegion().GetSize();
	auto iterator = ConstIteratorType(image, inputRegion);
	iterator.GoToBegin();

	int maxY = INT_MIN, maxX = INT_MIN;
	int minY = INT_MAX, minX = INT_MAX;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _foregroundValue)
		{
			if (iterator.GetIndex()[1] > maxY) maxY = iterator.GetIndex()[1];
			if (iterator.GetIndex()[0] > maxX) maxX = iterator.GetIndex()[0];

			if (iterator.GetIndex()[1] < minY) minY = iterator.GetIndex()[1];
			if (iterator.GetIndex()[0] < minX) minX = iterator.GetIndex()[0];
		}
		++iterator;
	}

	auto mediumY = minY + (maxY - minY) / 2;
	auto mediumX = minX + (maxX - minX) / 2;
	std::cout << "[MediumPoint] X = " << mediumX << "\t Y = " << mediumY << std::endl;
	DicomImageType::IndexType point = { mediumX, mediumY };
	return point;
}

DicomSeriesType::IndexType
DicomProcessing::GetMediumPointBelowZ(DicomSeriesType::Pointer image, DicomSeriesType::PixelType _foregroundValue, int aboveZ)
{
	using ConstIteratorType = itk::ImageRegionConstIterator<DicomSeriesType>;
	auto inputRegion = image->GetLargestPossibleRegion().GetSize();
	auto iterator = ConstIteratorType(image, inputRegion);
	iterator.GoToBegin();

	int maxZ = INT_MIN, maxY = INT_MIN, maxX = INT_MIN;
	int minZ = INT_MAX, minY = INT_MAX, minX = INT_MAX;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _foregroundValue && iterator.GetIndex()[2] < aboveZ)
		{
			if (iterator.GetIndex()[2] > maxZ) maxZ = iterator.GetIndex()[2];
			if (iterator.GetIndex()[1] > maxY) maxY = iterator.GetIndex()[1];
			if (iterator.GetIndex()[0] > maxX) maxX = iterator.GetIndex()[0];

			if (iterator.GetIndex()[2] < minZ) minZ = iterator.GetIndex()[2];
			if (iterator.GetIndex()[1] < minY) minY = iterator.GetIndex()[1];
			if (iterator.GetIndex()[0] < minX) minX = iterator.GetIndex()[0];
		}
		++iterator;
	}

	auto mediumZ = minZ + (maxZ - minZ) / 2;
	auto mediumY = minY + (maxY - minY) / 2;
	auto mediumX = minX + (maxX - minX) / 2;
	std::cout << "[MediumPointBelowZ] X = " << mediumX << "\t Y = "
		<< mediumY << "\t Z = " << mediumZ << std::endl;
	DicomSeriesType::IndexType point = { mediumX, mediumY, mediumZ };
	return point;
}


template <typename T>
itk::SmartPointer<T> DicomProcessing::Copy(itk::SmartPointer<T> image)
{
	auto duplicator = itk::ImageDuplicator<T>::New();
	duplicator->SetInputImage(image);
	duplicator->Update();

	return duplicator->GetOutput();
}

itk::SmartPointer<DicomImageType> Copy(itk::SmartPointer<DicomImageType> image)
{
	return DicomProcessing::Copy<DicomImageType>(image);
}
itk::SmartPointer<DicomSeriesType> Copy(itk::SmartPointer<DicomSeriesType> image)
{
	return DicomProcessing::Copy<DicomSeriesType>(image);
}

itk::SmartPointer<DicomImageType> DicomProcessing::BinaryInverse(
	itk::SmartPointer<DicomImageType> image, DicomImageType::PixelType foregroundValue)
{
	using ConstIteratorType = itk::ImageRegionConstIterator<DicomImageType>;
	auto volumeRegion = image->GetLargestPossibleRegion();
	auto volumeSize = volumeRegion.GetSize();
	auto volumeSpacing = image->GetSpacing();
	auto constIt = ConstIteratorType(image, volumeRegion);
	constIt.GoToBegin();

	// new projection image
	auto inversed = DicomImageType::New();
	auto region = DicomImageType::RegionType();
	DicomImageType::IndexType projectionStart = { 0, 0 };
	DicomImageType::SizeType projectionSize;
	DicomImageType::SpacingType imageSpacing = itk::Vector<double, 2U>();

	projectionSize = { volumeSize[0], volumeSize[1] };
	imageSpacing[0] = volumeSpacing[0];
	imageSpacing[1] = volumeSpacing[1];

	region.SetSize(projectionSize);
	region.SetIndex(projectionStart);
	inversed->SetRegions(region);
	inversed->Allocate();
	inversed->FillBuffer(0);
	inversed->SetSpacing(imageSpacing);

	while (!constIt.IsAtEnd())
	{
		if (constIt.Value() == foregroundValue)
		{
			inversed->SetPixel(constIt.GetIndex(), 0);
		}
		else
		{
			inversed->SetPixel(constIt.GetIndex(), foregroundValue);
		}

		++constIt;
	}

	return inversed;
}



double* DicomProcessing::CalculateTransformationMatrix(
	double* sourcePoints, double* sourceCenter,
	double* targetPoints, double* targetCenter, int count)
{
	using MatrixType = itk::VariableSizeMatrix<double>;
	MatrixType A, B;
	A.SetSize(3, count); B.SetSize(3, count);
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			A[j][i] = sourcePoints[3 * i + j] - sourceCenter[j];
			B[j][i] = targetPoints[3 * i + j] - targetCenter[j];
		}
	}

	MatrixType H; H.SetSize(3, 3);
	H = A * (B.GetTranspose());

	vnl_svd<double> solve(H.GetVnlMatrix());

	auto R = solve.V() * (solve.U().transpose());
	auto detR = vnl_determinant(R);
	if (detR < 0)
	{
		std::cout << "Determinant of R is negative: " << detR << std::endl;
		auto newV = solve.V();
		newV[0][2] = newV(0, 2) * (-1);
		newV[1][2] = newV(1, 2) * (-1);
		newV[2][2] = newV(2, 2) * (-1);
		R = newV * (solve.U().transpose());
	}

	std::cout << "Rot: " << std::endl;
	std::cout << R[0][0] << "," << R[0][1] << "," << R[0][2] << std::endl;
	std::cout << R[1][0] << "," << R[1][1] << "," << R[1][2] << std::endl;
	std::cout << R[2][0] << "," << R[2][1] << "," << R[2][2] << std::endl << std::endl;

	auto centrA = vnl_matrix<double>(3, 3);
	centrA[0][0] = sourceCenter[0]; centrA[1][0] = sourceCenter[1]; centrA[2][0] = sourceCenter[2];
	centrA[0][1] = sourceCenter[0]; centrA[1][1] = sourceCenter[1]; centrA[2][1] = sourceCenter[2];
	centrA[0][2] = sourceCenter[0]; centrA[1][2] = sourceCenter[1]; centrA[2][2] = sourceCenter[2];
	auto centrB = vnl_matrix<double>(3, 3);
	centrB[0][0] = targetCenter[0]; centrB[1][0] = targetCenter[1]; centrB[2][0] = targetCenter[2];
	centrB[0][1] = targetCenter[0]; centrB[1][1] = targetCenter[1]; centrB[2][1] = targetCenter[2];
	centrB[0][2] = targetCenter[0]; centrB[1][2] = targetCenter[1]; centrB[2][2] = targetCenter[2];
	
	auto T = centrB - R * centrA;

	std::cout << "Tran: " << std::endl;
	std::cout << T[0][0] << "," << T[0][1] << "," << T[0][2] << std::endl;
	std::cout << T[1][0] << "," << T[1][1] << "," << T[1][2] << std::endl;
	std::cout << T[2][0] << "," << T[2][1] << "," << T[2][2] << std::endl << std::endl;

	auto S = R;

	double* result = new double[16];
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			result[4* i + j] = S[i][j];
	result[3] = T(0, 0);
	result[7] = T(1, 1);
	result[11] = T(2,2);
	result[12] = 0;
	result[13] = 0;
	result[14] = 0;
	result[15] = 1;

	return result;
}