#include "MandibularBoneToCondylesAlgorithm.h"
#include "ITK-5.1/itkImageLinearIteratorWithIndex.h"

#define WITHCLOSINGFILTER
//#define LOGCLOSINGITERATIONS
//#define LOGALLCONDYLESINFO

MandibularBoneToCondylesAlgorithm::MandibularBoneToCondylesAlgorithm()
{
	keepNFilter = KeepNFilterType::New();
	leftRamusLineExtractor = RamusLineType::New();
	rightRamusLineExtractor = RamusLineType::New();
	leftNotchLineExtractor = NotchLineType::New();
	rightNotchLineExtractor = NotchLineType::New();

#ifdef WITHCLOSINGFILTER

	closingFilter = ClosingFilterType::New();
	ClosingKernelType::RadiusType closingRadius;
	for (unsigned int i = 0; i < ImageType::ImageDimension; i++)
		closingRadius[i] = closingFilterRadius;
	auto closingKernel = ClosingKernelType::Ball(closingRadius);
	closingFilter->SetRadius(closingRadius);

#endif
}

void MandibularBoneToCondylesAlgorithm::GenerateData()
{
	// expecting binary image with mandibular bone segment
	auto input = DicomSeriesType::New();
	input->Graft(const_cast<DicomSeriesType*>(this->GetInput()));

	// find mediums
	_mediumPoint = DicomProcessing::GetMediumPoint(input, _foregroundValue);
	auto mediumY = _mediumPoint[1];
	auto mediumZ = _mediumPoint[2];

	using IteratorType = itk::ImageRegionIterator<ImageType>;
	auto inputRegion = input->GetLargestPossibleRegion();
	auto iterator = IteratorType(input, inputRegion);
	iterator.GoToBegin();

	// remove half of frontal - bottom voxels of mandibula
	int count = 0;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _foregroundValue)
		{
			if (iterator.GetIndex()[1] < mediumY
				&& iterator.GetIndex()[2] < mediumZ)
			{
				iterator.Set(_backgroundValue);
				count++;
			}
		}
		++iterator;
	}
#ifdef LOGALLCONDYLESINFO
	std::cout << "Removed mandibular bones voxel count: " << count << std::endl;
#endif // LOGALLCONDYLESINFO


	// keep 2 bigest objects (mandibula ramuses)
	keepNFilter->SetForegroundValue(_foregroundValue);
	keepNFilter->SetInput(input);
	keepNFilter->SetNumberOfObjects(2);
	keepNFilter->Update();

	boundingBoxXYZ = DicomProcessing::GetSelectionBoundingBoxXYZ(keepNFilter->GetOutput(), _foregroundValue);
	boundingRegion = GetBoundedRegion();

#ifdef LOGALLCONDYLESINFO
	std::cout << "Preparing condyle points..." << std::endl;
#endif // LOGALLCONDYLESINFO

	leftCondylePoints = GetCondyle(keepNFilter->GetOutput(), CondyleType::Left);
	rightCondylePoints = GetCondyle(keepNFilter->GetOutput(), CondyleType::Right);
	leftCondyleCenter = GetPointsGeometricalCenter(leftCondylePoints);
	rightCondyleCenter = GetPointsGeometricalCenter(rightCondylePoints);

	input->FillBuffer(_backgroundValue);
	for (int i = 0; i < leftCondylePoints.size(); i++)
		input->SetPixel(leftCondylePoints[i], 1024);

	for (int i = 0; i < rightCondylePoints.size(); i++)
		input->SetPixel(rightCondylePoints[i], 1024);

	auto closingIterationProgressStep = 1.0 / (3 + closingIterations);
	auto progress = 3 * closingIterationProgressStep;
	this->UpdateProgress(progress);

#ifdef WITHCLOSINGFILTER

	closingFilter->SetForegroundValue(1024);
	auto closingInput = input;
	for (int i = 0; i < closingIterations; i++)
	{
#ifdef LOGCLOSINGITERATIONS
		std::cout << "Closing iteration: " << i + 1 << std::endl;
#endif // LOGCLOSINGITERATIONS

		closingFilter->SetInput(closingInput);
		closingFilter->Update();
		if (i != closingIterations - 1)
		{
			closingInput = DicomProcessing::Copy<DicomSeriesType>(closingFilter->GetOutput());
			closingFilter->SetInput(closingInput);
			closingFilter->ResetPipeline();
		}
		progress += closingIterationProgressStep;
		this->UpdateProgress(progress);
	}
#endif

	auto result = closingFilter->GetOutput();
	// filling
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto inputRegion2 = boundingRegion;
	auto iterator2 = ConstIteratorType(result, inputRegion2);
	iterator2.GoToBegin();

	leftCondylePoints.clear();
	rightCondylePoints.clear();
	auto mediumX = _mediumPoint[0];
	while (!iterator2.IsAtEnd())
	{
		if (iterator2.Value() == 1024)
		{
			if (iterator2.GetIndex()[0] < mediumX)
				leftCondylePoints.push_back(iterator2.GetIndex());
			else if (iterator2.GetIndex()[0] > mediumX)
				rightCondylePoints.push_back(iterator2.GetIndex());
		}
		++iterator2;
	}


	for (int i = 0; i < leftCondylePoints.size(); i++)
	{
		auto curr = leftCondylePoints[i];
		unsigned int min = curr[1], max = curr[1];
		for (int j = 0; j < leftCondylePoints.size(); j++)
		{
			auto next = leftCondylePoints[j];
			if (next[0] != curr[0] || next[2] != curr[2]) continue;

			if (next[1] < min)
				min = next[1];
			if (next[1] > max)
				max = next[1];
		}

		for (unsigned int k = min + 1; k < max; k++)
		{
			auto in = itk::Index <3U>{ curr[0], k, curr[2] };
			if (result->GetPixel(in) == 1024)
			{

			}
			else
			{
				leftCondylePoints.push_back(in);
				result->SetPixel(in, 1024);
			}
		}
	}

	for (int i = 0; i < rightCondylePoints.size(); i++)
	{
		auto curr = rightCondylePoints[i];
		unsigned int min = curr[1], max = curr[1];
		for (int j = 0; j < rightCondylePoints.size(); j++)
		{
			auto next = rightCondylePoints[j];
			if (next[0] != curr[0] || next[2] != curr[2]) continue;

			if (next[1] < min)
				min = next[1];
			if (next[1] > max)
				max = next[1];
	}

		for (unsigned int k = min + 1; k < max; k++)
		{
			auto in = itk::Index <3U>{ curr[0], k, curr[2] };
			if (result->GetPixel(in) == 1024)
			{

			}
			else
			{
				rightCondylePoints.push_back(in);
				result->SetPixel(in, 1024);
			}
		}
	}

#ifdef WITHCLOSINGFILTER
	this->GraftOutput(result);
#else
	this->GraftOutput(input);
#endif

	this->UpdateProgress(1.0);
}

void MandibularBoneToCondylesAlgorithm::SaveLineImage(DicomImage* image, std::vector<double> line)
{
	auto tolerance = line[0] < 1 ? 1 : line[0] < 5 ? 2 : line[0] < 20 ? 3 : 4;
	using IteratorType = itk::ImageRegionIterator<DicomImageType>;
	auto itkImage = image->GetItkDicomImage();
	auto it = IteratorType(itkImage, itkImage->GetLargestPossibleRegion());
	it.GoToBegin();
	while (!it.IsAtEnd())
	{
		auto in = it.GetIndex();
		auto val = line[0] * in[0] + line[1];
		if (in[1] >= val - tolerance && in[1] <= val + tolerance)
			it.Set(25000);
		++it;
	}

	image->SetItkDicomImage(itkImage);
	auto name = image->GetFileName();
	auto path = "C:\\temp\\" + name + " - line.dcm";
	image->Save(path.c_str());
}

void MandibularBoneToCondylesAlgorithm::SaveAllLinesImage(
	DicomImage* targetImage, std::vector<std::vector<double>> lines)
{
	using IteratorType = itk::ImageRegionIterator<DicomImageType>;
	auto itkImage = targetImage->GetItkDicomImage();
	auto it = IteratorType(itkImage, itkImage->GetLargestPossibleRegion());

	for (int i = 0; i < lines.size(); i++)
	{
		auto line = lines[i];
		auto tolerance = line[0] < 1 ? 1 : line[0] < 5 ? 2 : line[0] < 20 ? 3 : 4;
		it.GoToBegin();
		while (!it.IsAtEnd())
		{
			auto in = it.GetIndex();
			auto val = line[0] * in[0] + line[1];
			if (in[1] >= val - tolerance && in[1] <= val + tolerance)
				it.Set(25000);
			++it;
		}
	}
	targetImage->SetItkDicomImage(itkImage);
}

std::vector<MandibularBoneToCondylesAlgorithm::ImageType::IndexType>
MandibularBoneToCondylesAlgorithm::GetCondyle(ImageType::Pointer image, CondyleType condyleType)
{
	std::vector<ImageType::IndexType> points = std::vector<ImageType::IndexType>();
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto inputRegion = boundingRegion;
	auto iterator = ConstIteratorType(image, inputRegion);
	iterator.GoToBegin();

	auto mediumX = _mediumPoint[0];
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _foregroundValue)
		{
			if (condyleType == CondyleType::Left)
			{
				if (iterator.GetIndex()[0] < mediumX)
					points.push_back(iterator.GetIndex());
			}
			else
			{
				if (iterator.GetIndex()[0] > mediumX)
					points.push_back(iterator.GetIndex());
			}
		}
		++iterator;
	}

	
	auto projectionImage = GetProjection(image, points, condyleType);
	projectionImage->Swap();
	auto contoursImage = GetLeftContours(projectionImage);
	auto ramusLineExtractor = condyleType == CondyleType::Left ? leftRamusLineExtractor : rightRamusLineExtractor;
	ramusLineExtractor->SetImage(contoursImage->GetItkDicomImage());
	ramusLineExtractor->SetForegroundValue(_foregroundValue);
	ramusLineExtractor->SetIterations(25);
	ramusLineExtractor->Compute();
	auto ramusLine = ramusLineExtractor->GetRamusLine();
	delete contoursImage;

	auto topContoursImage = GetTopContours(projectionImage);
	auto notchLineExtractor = condyleType == CondyleType::Left ? leftNotchLineExtractor : rightNotchLineExtractor;
	notchLineExtractor->SetImage(topContoursImage->GetItkDicomImage());
	notchLineExtractor->SetForegroundValue(_foregroundValue);
	notchLineExtractor->SetRamusLine(ramusLine);
	notchLineExtractor->Compute();
	auto notchLine = notchLineExtractor->GetNotchLine();
	auto notchPoint = notchLineExtractor->GetNotchPoint();
	auto notchRamusLine = notchLineExtractor->GetNotchRamusLine();
	delete topContoursImage;

	auto notchLineMinimalSection = GetNotchRamusLineWithMinimalSection(notchLine, points);

	SaveAllLinesImage(projectionImage,
		std::vector<std::vector<double>> { ramusLine, notchLine, notchRamusLine, notchLineMinimalSection });

	// swap lines and point for cutting the volume
	auto projectionSize = projectionImage->GetItkDicomImage()->GetLargestPossibleRegion().GetSize();
	notchPoint[0] = projectionSize[0] - notchPoint[0];
	notchPoint[1] = projectionSize[1] - notchPoint[1];
	auto notchEdgeVal = (projectionSize[1] - notchLine[1]) / notchLine[0];
	auto notchMinimalSectionEdgeVal = (projectionSize[1] - notchLineMinimalSection[1]) / notchLineMinimalSection[0];
	auto ramusEdgeVal = (projectionSize[1] - ramusLine[1]) / ramusLine[0];
	auto notchRamusEdgeVal = (projectionSize[1] - notchRamusLine[1]) / notchRamusLine[0];
	auto notchRatio = (projectionSize[0] - notchEdgeVal) / notchEdgeVal;
	auto notchMinimalSectionRatio = (projectionSize[0] - notchMinimalSectionEdgeVal) / notchMinimalSectionEdgeVal;
	auto ramusRatio = (projectionSize[0] - ramusEdgeVal) / ramusEdgeVal;
	auto notchRamusRatio = (projectionSize[0] - notchRamusEdgeVal) / notchRamusEdgeVal;
	notchLine[1] = (notchLine[1] - projectionSize[1]) * notchRatio;
	notchLineMinimalSection[1] = (notchLineMinimalSection[1] - projectionSize[1]) * notchMinimalSectionRatio;
	notchRamusLine[1] = (notchRamusLine[1] - projectionSize[1]) * notchRamusRatio;
	ramusLine[1] = (ramusLine[1] - projectionSize[1]) * ramusRatio;

	
	std::vector<ImageType::IndexType> condylePoints = std::vector<ImageType::IndexType>();
	iterator.GoToBegin();
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _foregroundValue)
		{
			bool shouldContinue = true;
			if (condyleType == CondyleType::Left)
			{
				shouldContinue = iterator.GetIndex()[0] < mediumX;
			}
			else
			{
				shouldContinue = iterator.GetIndex()[0] > mediumX;
			}

			if (shouldContinue)
			{
				auto in = iterator.GetIndex();
				auto val1 = ramusLine[0] * in[1] + ramusLine[1];
				auto val2 = notchLine[0] * in[1] + notchLine[1];
				auto val3 = notchRamusLine[0] * in[1] + notchRamusLine[1];
				auto val4 = notchLineMinimalSection[0] * in[1] + notchLineMinimalSection[1];
				if (in[2] > val1 && in[2] > val2 && in[2] < val3 && in[2] > val4)
					condylePoints.push_back(in);
			}
		}
		++iterator;
	}

	std::cout << "[CondyleCut] Condyle points: " << condylePoints.size() << std::endl;
	return condylePoints;
}

DicomImage* MandibularBoneToCondylesAlgorithm::GetProjection(ImageType::Pointer volumeImage, std::vector<ImageType::IndexType> points, CondyleType condyleType)
{
	auto volumeRegion = volumeImage->GetLargestPossibleRegion();
	auto volumeSize = volumeRegion.GetSize();
	auto volumeSpacing = volumeImage->GetSpacing();

	auto dicomImage = DicomImageType::New();
	auto region = DicomImageType::RegionType();
	DicomImageType::IndexType projectionStart = { 0, 0 };
	DicomImageType::SizeType projectionSize = { volumeSize[1], volumeSize[2] }; // YZ
	region.SetSize(projectionSize);
	region.SetIndex(projectionStart);
	dicomImage->SetRegions(region);
	dicomImage->Allocate();
	dicomImage->FillBuffer(0);
	DicomImageType::SpacingType imageSpacing = itk::Vector<double, 2U>();
	imageSpacing[0] = volumeSpacing[1];
	imageSpacing[1] = volumeSpacing[2];
	dicomImage->SetSpacing(imageSpacing);

	auto width = projectionSize[0];
	auto height = projectionSize[1];
	for (auto it = points.begin(); it != points.end(); it++)
	{
		auto index = *it;
		DicomImageType::IndexType projectedIndex = { index[1], index[2] };
		dicomImage->SetPixel(projectedIndex, _foregroundValue);
	}
	auto name = condyleType == Left ? "Mandibular_LeftLinesImage" : "Mandibular_RightLinesImage";
	if (condyleType == Left)
	{
		leftLinesImage = new DicomImage();
		leftLinesImage->SetFileName(name);
		leftLinesImage->SetItkDicomImage(dicomImage);
		return leftLinesImage;
	}
	else
	{
		rightLinesImage = new DicomImage();
		rightLinesImage->SetFileName(name);
		rightLinesImage->SetItkDicomImage(dicomImage);
		return rightLinesImage;
	}
}

DicomImage* MandibularBoneToCondylesAlgorithm::GetLeftContours(DicomImage* dicomImage)
{
#ifdef LOGALLCONDYLESINFO
	std::cout << "Preparing left contours image..." << std::endl;
#endif // LOGALLCONDYLESINFO

	auto itkImage = dicomImage->GetItkDicomImage();

	auto itkContoursImage = DicomImageType::New();
	itkContoursImage->SetRegions(itkImage->GetLargestPossibleRegion());
	itkContoursImage->Allocate();
	itkContoursImage->FillBuffer(0);
	itkContoursImage->SetSpacing(itkImage->GetSpacing());

	using ConstIteratorType = itk::ImageLinearConstIteratorWithIndex<DicomImageType>;
	using IteratorType = itk::ImageLinearIteratorWithIndex<DicomImageType>;
	auto region = itkImage->GetLargestPossibleRegion();
	auto iterator = ConstIteratorType(itkImage, region);
	iterator.GoToBegin();

	auto contoursRegion = itkContoursImage->GetLargestPossibleRegion();
	auto contoursIterator = IteratorType(itkContoursImage, contoursRegion);
	contoursIterator.GoToBegin();

	for ( ; !iterator.IsAtEnd(); iterator.NextLine(), contoursIterator.NextLine())
	{
		while (!iterator.IsAtEndOfLine())
		{
			if (iterator.Value() == _foregroundValue)
			{
				contoursIterator.Set(_foregroundValue);
				iterator.GoToEndOfLine();
				contoursIterator.GoToEndOfLine();
			}
			++iterator;
			++contoursIterator;
		}
	}

	auto contoursImage = new DicomImage();
	contoursImage->SetItkDicomImage(itkContoursImage);
	//auto name = dicomImage->GetFileName() + "_contour";
	//contoursImage->SetFileName(name.c_str());
	//auto path = "C:\\temp\\" + name + ".dcm";
	//std::cout << "Saving contours to: " << path << std::endl;
	//contoursImage->Resample();
	//contoursImage->Save(path.c_str());
	//std::cout << "Saving done." << std::endl;

	return contoursImage;
}

DicomImage* MandibularBoneToCondylesAlgorithm::GetTopContours(DicomImage* dicomImage)
{
#ifdef LOGALLCONDYLESINFO
	std::cout << "Preparing top contours image..." << std::endl;
#endif // LOGALLCONDYLESINFO

	auto itkImage = dicomImage->GetItkDicomImage();

	auto itkContoursImage = DicomImageType::New();
	itkContoursImage->SetRegions(itkImage->GetLargestPossibleRegion());
	itkContoursImage->Allocate();
	itkContoursImage->FillBuffer(0);
	itkContoursImage->SetSpacing(itkImage->GetSpacing());

	using ConstIteratorType = itk::ImageLinearConstIteratorWithIndex<DicomImageType>;
	using IteratorType = itk::ImageLinearIteratorWithIndex<DicomImageType>;
	auto region = itkImage->GetLargestPossibleRegion();
	auto iterator = ConstIteratorType(itkImage, region);
	iterator.GoToBegin();
	iterator.SetDirection(1);

	auto contoursRegion = itkContoursImage->GetLargestPossibleRegion();
	auto contoursIterator = IteratorType(itkContoursImage, contoursRegion);
	contoursIterator.GoToBegin();
	contoursIterator.SetDirection(1);

	for (; !iterator.IsAtEnd(); iterator.NextLine(), contoursIterator.NextLine())
	{
		while (!iterator.IsAtEndOfLine())
		{
			if (iterator.Value() == _foregroundValue)
			{
				contoursIterator.Set(_foregroundValue);
				iterator.GoToEndOfLine();
				contoursIterator.GoToEndOfLine();
			}
			++iterator;
			++contoursIterator;
		}
	}

	auto contoursImage = new DicomImage();
	contoursImage->SetItkDicomImage(itkContoursImage);
	//auto name = dicomImage->GetFileName() + "_top contour";
	//contoursImage->SetFileName(name.c_str());
	//auto path = "C:\\temp\\" + name + ".dcm";
	//std::cout << "Saving contours to: " << path << std::endl;
	//contoursImage->Resample();
	//contoursImage->Save(path.c_str());
	//std::cout << "Saving done." << std::endl;

	return contoursImage;
}

MandibularBoneToCondylesAlgorithm::ImageType::RegionType
MandibularBoneToCondylesAlgorithm::GetBoundedRegion()
{
	ImageType::RegionType inputRegion;

	ImageType::RegionType::IndexType inputStart;
	ImageType::RegionType::SizeType  size;

	inputStart[0] = boundingBoxXYZ[0][0];
	inputStart[1] = boundingBoxXYZ[0][1];
	inputStart[2] = boundingBoxXYZ[0][2];

	size[0] = boundingBoxXYZ[1][0] - inputStart[0];
	size[1] = boundingBoxXYZ[1][1] - inputStart[1];
	size[2] = boundingBoxXYZ[1][2] - inputStart[2];

	inputRegion.SetSize(size);
	inputRegion.SetIndex(inputStart);

	return inputRegion;
}


MandibularBoneToCondylesAlgorithm::ImageType::IndexType
MandibularBoneToCondylesAlgorithm::GetPointsGeometricalCenter(
	std::vector<ImageType::IndexType> points)
{
	int xSum = 0, ySum = 0, zSum = 0;
	int count = points.size();
	for (int i = 0; i < count; i++)
	{
		xSum += points[i][0];
		ySum += points[i][1];
		zSum += points[i][2];
	}
	xSum /= count;
	ySum /= count;
	zSum /= count;

	return ImageType::IndexType { xSum, ySum, zSum };
}


std::vector<double>
MandibularBoneToCondylesAlgorithm::GetNotchRamusLineWithMinimalSection(
	std::vector<double> notchLine, std::vector<ImageType::IndexType> allPoints)
{
	// get notch line with higher B component which represents minimal cross section
	double previousSection = DBL_MAX;
	double currentSection = DBL_MAX;

	auto imageSize = this->GetInput()->GetLargestPossibleRegion().GetSize();
	double B = (-1) * 3;
	const double step = (-1) * 0.1;
	bool found = false;
	while(!found)
	{
		auto sectionVoxels = std::vector<ImageType::IndexType>();
		for (int i = 0; i < allPoints.size(); i++)
		{
			auto in = allPoints[i];
			ImageType::IndexType swapped = { imageSize[0] - in[0] - 1, imageSize[1] - in[1] - 1, imageSize[2] - in[2] - 1 };
			auto val = notchLine[0] * swapped[1] + notchLine[1] + B;
			if (std::abs(swapped[2] - val) <= 2)
				sectionVoxels.push_back(swapped);
		}

		if (sectionVoxels.size() == 0) break;
		
		// TODO?
		//currentSection = GetArea(sectionVoxels);
		//std::cout << "[CondyleCut] B: " << B << "\t Voxels: " << sectionVoxels.size() << "\t Area: " << currentSection << std::endl;

		//found = currentSection > previousSection;
		//if (found)
		//{
		//	B -= step; // previous
		//	break;
		//}
		//else if (currentSection == 0)
		//{
		//	previousSection = 0;
		//	break;
		//}

		//previousSection = currentSection;
		B += step;
	}

	auto minimalSection = 0; // previousSection;
	std::cout << "[CondyleCut] Final: B: " << B / 2 << "\t Area: " << minimalSection << "\t Line: y=" << notchLine[0]
		<< "x + " << notchLine[1] + B / 2 << std::endl;

	auto newLine = std::vector<double>{ notchLine[0], notchLine[1] + B / 2 };
	return newLine;
}

double MandibularBoneToCondylesAlgorithm::GetArea(std::vector<ImageType::IndexType> points)
{
	// geometric center
	double x = 0, y = 0, z = 0;
	for (int i = 0; i < points.size(); i++)
	{
		x += points[i][0];
		y += points[i][1];
		z += points[i][2];
	}
	x /= points.size();
	y /= points.size();
	z /= points.size();

	ImageType::IndexType maxPoint;
	double maxDist = 0;;
	for (int i = 0; i < points.size(); i++)
	{
		auto pt = points[i];
		auto dist = std::sqrt(
			std::pow(x - pt[0], 2) + std::pow(y - pt[1], 2) + std::pow(z - pt[2], 2));
		if (dist > maxDist)
		{
			maxDist = dist;
			maxPoint = pt;
		}
	}

	return itk::Math::pi * maxDist * maxDist;
}

MandibularBoneToCondylesAlgorithm::~MandibularBoneToCondylesAlgorithm()
{
	std::cout << "BoneToCondyles dctor... ";

	if (leftLinesImage != nullptr)
		leftLinesImage = nullptr;
	if (rightLinesImage != nullptr)
		rightLinesImage = nullptr;

	std::cout << "done." << std::endl;
}