#include "NasionPointExtractor.h"
#include "helpers.h"

void NasionPointExtractor::SetImage(ImageType::Pointer image)
{
	this->image = new DicomSeries();
	this->image->SetItkImageData(DicomProcessing::Copy<ImageType>(image));
}

void NasionPointExtractor::Compute()
{
	CreateYZProjection();
	CalculateNasion();
	SelectNasionLevel();
}

void NasionPointExtractor::CreateYZProjection()
{
	auto sourceImage = image->GetItkImageData();
	auto volumeRegion = sourceImage->GetLargestPossibleRegion();
	auto volumeSize = volumeRegion.GetSize();
	auto volumeSpacing = sourceImage->GetSpacing();

	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto constIt = ConstIteratorType(sourceImage, volumeRegion);
	constIt.GoToBegin();

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

	while (!constIt.IsAtEnd())
	{
		if (constIt.Value() == foregroundValue && constIt.GetIndex()[2] >= zThreshold)
		{
			// set projection image pixel
			auto index = constIt.GetIndex();

			DicomImageType::IndexType projectedIndex;
			projectedIndex = { index[1], index[2] };

			dicomImage->SetPixel(projectedIndex, foregroundValue);
		}
		++constIt;
	}
	
	this->upperSkullProjection = new DicomImage();
	upperSkullProjection->SetItkDicomImage(dicomImage);
	upperSkullProjection->Swap();
}

void NasionPointExtractor::CalculateNasion()
{
	auto nasionPointYZ = GetMinimalYProjectionPoint();
	nasionPoint = GetNasionPointFromProjected(nasionPointYZ);
	std::cout << "[NasionPoint] " << nasionPoint[0] << ", " << nasionPoint[1] << ", " << nasionPoint[2] << std::endl;
}

void NasionPointExtractor::SelectNasionLevel()
{
	auto sourceRegion = image->GetItkImageData()->GetLargestPossibleRegion();
	auto sourceSize = sourceRegion.GetSize();
	auto nasionZ = sourceSize[2] - nasionPoint[2] - 1;
	auto width = upperSkullProjection->GetWidth();

	auto image = upperSkullProjection->GetItkDicomImage();
	for (int i = 0; i < width; i++)
	{
		DicomImageType::IndexType in = { i, nasionZ };
		image->SetPixel(in, 25000);
	}
	upperSkullProjection->SetItkDicomImage(image);
	upperSkullProjection->SetFileName("Mandibular_UpperSkullProjection");
}

DicomImageType::IndexType NasionPointExtractor::GetMinimalYProjectionPoint()
{
	auto sourceImage = upperSkullProjection->GetItkDicomImage();
	auto sourceRegion = sourceImage->GetLargestPossibleRegion();
	auto sourceSize = sourceRegion.GetSize();
	using ConstIteratorType = itk::ImageLinearConstIteratorWithIndex<DicomImageType>;
	auto constIt = ConstIteratorType(sourceImage, sourceRegion);
	constIt.GoToBegin();

	std::cout << "[NasionPoint] Extracting nasal bone point... Threshold Z: " << (sourceSize[1] - zThreshold) << std::endl;

	// find nasal bone maximum
	DicomImageType::IndexType maximal;
	int maximalY = 0;
	while (!constIt.IsAtEnd())
	{
		constIt.GoToReverseBeginOfLine();

		auto in = constIt.GetIndex();
		if (in[1] >= (sourceSize[1] - zThreshold)) break;

		while (!constIt.IsAtReverseEndOfLine())
		{
			in = constIt.GetIndex();
			if (constIt.Value() == foregroundValue)
			{
				if (in[0] >= maximalY)
				{
					maximalY = in[0];
					maximal = in;
				}
				break;
			}
			--constIt;
		}

		constIt.NextLine();
	}

	std::cout << "[NasionPoint] Projected nasal bone maximum point: " << maximal[0] << ", " << maximal[1] << std::endl;

	constIt.GoToBegin();
	std::cout << "[NasionPoint] Extracting nasion point..." << std::endl;

	// find minimum above nasal bone maximum
	DicomImageType::IndexType minimal;
	std::vector<DicomImageType::IndexType> minimals;
	int minimalY = sourceRegion.GetSize()[0];
	while (!constIt.IsAtEnd())
	{
		constIt.GoToReverseBeginOfLine();

		auto in = constIt.GetIndex();
		if (in[1] >= maximal[1]) break;

		while (!constIt.IsAtReverseEndOfLine())
		{
			in = constIt.GetIndex();
			if (constIt.Value() == foregroundValue)
			{
				if (in[0] <= minimalY)
				{
					if (in[0] < minimalY)
						minimals.clear();

					minimalY = in[0];
					
					minimals.push_back(in);
				}
				break;
			}
			--constIt;
		}

		constIt.NextLine();
	}

	unsigned int meanZ = 0;
	for (int i = 0; i < minimals.size(); i++)
		meanZ += minimals[i][1];
	meanZ /= minimals.size();

	minimal = DicomImageType::IndexType{ minimalY, meanZ };

	std::cout << "[NasionPoint] Projected nasion point: " << minimal[0] << ", " << minimal[1] << std::endl;
	return minimal;
}

NasionPointExtractor::IndexType
NasionPointExtractor::GetNasionPointFromProjected(DicomImageType::IndexType projected)
{
	IndexType targetPoint;

	auto sourceImage = image->GetItkImageData();
	auto sourceRegion = sourceImage->GetLargestPossibleRegion();
	auto sourceSize = sourceRegion.GetSize();
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto constIt = ConstIteratorType(sourceImage, sourceRegion);
	constIt.GoToBegin();

	DicomImageType::IndexType swapped = { sourceSize[1] - projected[0] - 1, sourceSize[2] - projected[1] - 1 };
	std::vector<IndexType> targetPoints;
	while (!constIt.IsAtEnd())
	{
		auto in = constIt.GetIndex();
		if (constIt.Value() == foregroundValue
			&& in[1] == swapped[0]
			&& in[2] == swapped[1])
		{
			targetPoints.push_back(in);
		}

		++constIt;
	}

	if (targetPoints.size() == 0)
		std::cout << "[NasionPoint] ERROR: No target point found." << std::endl;
	else if (targetPoints.size() == 1)
	{
		std::cout << "[NasionPoint] Single target point: " << targetPoint[0]
			<< ", " << targetPoint[1] << ", " << targetPoint[2] << std::endl;
		targetPoint = targetPoints[0];
	}
	else
	{
		std::cout << "[NasionPoint] Many target points: Count = " << targetPoints.size() << std::endl;
		// get median
		std::vector<int> xcoords;
		for (int i = 0; i < targetPoints.size(); i++)
			xcoords.push_back(targetPoints[i][0]);

		std::sort(xcoords.begin(), xcoords.end());
		auto medianX = xcoords[xcoords.size() / 2];

		for (int i = 0; i < targetPoints.size(); i++)
		{
			if (targetPoints[i][0] == medianX)
				targetPoint = targetPoints[i];
			std::cout << "[NasionPoint] Pt" << i << " " << targetPoints[i][0]
				<< ", " << targetPoints[i][1] << ", " << targetPoints[i][2] << std::endl;
		}
	}

	return targetPoint;
}

NasionPointExtractor::~NasionPointExtractor()
{
	if (image != nullptr)
	{
		delete image;
		image = nullptr;
	}
	if (upperSkullProjection != nullptr)
		upperSkullProjection = nullptr;
}