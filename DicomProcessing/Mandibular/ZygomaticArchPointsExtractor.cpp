#include "ZygomaticArchPointsExtractor.h"
#include "helpers.h"

void ZygomaticArchPointsExtractor::SetImage(DicomSeriesType::Pointer image)
{
	this->image = new DicomSeries();
	this->image->SetItkImageData(image);
}

void ZygomaticArchPointsExtractor::Compute()
{
	projection = CreateTopProjection(image);
	archImage = ExtractZygomaticArchs(projection, image);

	auto rotatedProjection = GetRotatedImage(projection);
	auto rotatedArchs = GetRotatedImage3D(archImage);
	auto rotatedArchProjection = CreateTopProjection(rotatedArchs);

	auto tempFolder = std::string(helpers::GetAppDataTempPath());
	rotatedProjection->SaveCompressed((tempFolder + "\\RotatedProjection.jpg").c_str());
	rotatedArchProjection->SaveCompressed((tempFolder + "\\RotatedArchsProjeciton.jpg").c_str());

	auto pre = ExtractZygomaticPoints(rotatedArchs);
	
	auto inverseTransform = rotateMatrix3D;
	zygomaticPoints =
	{
		IndexType
		{
			(int)(pre[0][0] * inverseTransform[0][0] + pre[0][1] * inverseTransform[0][1] + pre[0][2] * inverseTransform[0][2]),
			(int)(pre[0][0] * inverseTransform[1][0] + pre[0][1] * inverseTransform[1][1] + pre[0][2] * inverseTransform[1][2]),
			(int)(pre[0][0] * inverseTransform[2][0] + pre[0][1] * inverseTransform[2][1] + pre[0][2] * inverseTransform[2][2])
		},
		IndexType
		{
			(int)(pre[1][0] * inverseTransform[0][0] + pre[1][1] * inverseTransform[0][1] + pre[1][2] * inverseTransform[0][2]),
			(int)(pre[1][0] * inverseTransform[1][0] + pre[1][1] * inverseTransform[1][1] + pre[1][2] * inverseTransform[1][2]),
			(int)(pre[1][0] * inverseTransform[2][0] + pre[1][1] * inverseTransform[2][1] + pre[1][2] * inverseTransform[2][2])
		}
	};

	std::cout << "[Zygomatic] Left point: " << zygomaticPoints[0][0] << "," << zygomaticPoints[0][1] << "," << zygomaticPoints[0][2] << std::endl;
	std::cout << "[Zygomatic] Right point: " << zygomaticPoints[1][0] << "," << zygomaticPoints[1][1] << "," << zygomaticPoints[1][2] << std::endl;
}

std::vector<DicomSeriesType::IndexType>
ZygomaticArchPointsExtractor::ExtractZygomaticPoints(DicomSeries* zygomaticImage)
{
	unsigned int minX = UINT_MAX;
	unsigned int maxX = 0;
	IndexType leftPoint;
	IndexType rightPoint;
	std::vector<IndexType> minXPoints;
	std::vector<IndexType> maxXPoints;

	auto itkImage = zygomaticImage->GetItkImageData();
	using ConstIteratorType = itk::ImageRegionIterator<DicomSeriesType>;
	auto constIt = ConstIteratorType(itkImage, itkImage->GetLargestPossibleRegion());
	constIt.GoToBegin();

	while (!constIt.IsAtEnd())
	{
		if (constIt.Get() == foregroundValue)
		{
			auto in = constIt.GetIndex();
			if (in[0] <= minX)
			{
				if (in[0] < minX)
					minXPoints.clear();

				minX = in[0];
				
				minXPoints.push_back(in);
			}
			else if (in[0] >= maxX)
			{
				if (in[0] > maxX)
					maxXPoints.clear();

				maxX = in[0];

				maxXPoints.push_back(in);
			}
		}

		++constIt;
	}

	unsigned int meanMinY = 0, meanMinZ = 0;
	for (int i = 0; i < minXPoints.size(); i++)
	{
		itkImage->SetPixel(minXPoints[i], 3000);
		meanMinY += minXPoints[i][1];
		meanMinZ += minXPoints[i][2];
	}
	meanMinY /= minXPoints.size();
	meanMinZ /= minXPoints.size();

	unsigned int meanMaxY = 0, meanMaxZ = 0;
	for (int i = 0; i < maxXPoints.size(); i++)
	{
		itkImage->SetPixel(maxXPoints[i], 3000);
		meanMaxY += maxXPoints[i][1];
		meanMaxZ += maxXPoints[i][2];
	}
	meanMaxY /= maxXPoints.size();
	meanMaxZ /= maxXPoints.size();

	leftPoint = IndexType{ minX, meanMinY, meanMinZ };
	rightPoint = IndexType{ maxX, meanMaxY, meanMaxZ };

	std::vector<IndexType> points;
	points.push_back(leftPoint);
	points.push_back(rightPoint);

	std::cout << "[Zygomatic] Left point rotated: " << leftPoint[0] << "," << leftPoint[1] << "," << leftPoint[2] << "\t Mean from " << minXPoints.size() << " points." << std::endl;
	std::cout << "[Zygomatic] Right point rotated: " << rightPoint[0] << "," << rightPoint[1] << "," << rightPoint[2] << "\t Mean from " << maxXPoints.size() << " points." << std::endl;

	return points;
}

DicomImage* ZygomaticArchPointsExtractor::CreateTopProjection(DicomSeries* image)
{
	auto itkImage = image->GetItkImageData();
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto volumeRegion = itkImage->GetLargestPossibleRegion();
	auto volumeSize = volumeRegion.GetSize();
	auto volumeSpacing = itkImage->GetSpacing();
	auto constIt = ConstIteratorType(itkImage, volumeRegion);
	constIt.GoToBegin();

	// new projection image
	auto dicomImage = DicomImageType::New();
	auto region = DicomImageType::RegionType();
	DicomImageType::IndexType projectionStart = { 0, 0 };
	DicomImageType::SizeType projectionSize;
	DicomImageType::SpacingType imageSpacing = itk::Vector<double, 2U>();

	projectionSize = { volumeSize[0], volumeSize[1] };
	imageSpacing[0] = volumeSpacing[0];
	imageSpacing[1] = volumeSpacing[1];

	region.SetSize(projectionSize);
	region.SetIndex(projectionStart);
	dicomImage->SetRegions(region);
	dicomImage->Allocate();
	dicomImage->FillBuffer(0);
	dicomImage->SetSpacing(imageSpacing);

	while (!constIt.IsAtEnd())
	{
		if (constIt.Value() == foregroundValue)
		{
			// set projection image pixel
			auto index = constIt.GetIndex();
			if (index[2] <= nasion[2] && index[1] >= nasion[1])
			{
				DicomImageType::IndexType projectedIndex;
				projectedIndex = { index[0], index[1] };
				dicomImage->SetPixel(projectedIndex, foregroundValue);
			}
		}
		++constIt;
	}

	auto projection = new DicomImage();
	projection->SetItkDicomImage(dicomImage);
	projection->CopyInformation(image);
	return projection;
}

DicomSeries* ZygomaticArchPointsExtractor::ExtractZygomaticArchs(
	DicomImage* projection, DicomSeries* input)
{
	auto itkImage = projection->GetItkDicomImage();
	auto size = itkImage->GetLargestPossibleRegion().GetSize();

	auto closing = ClosingType::New();
	auto radius = ClosingKernelType::RadiusType();
	radius[0] = 2; radius[1] = 2;
	auto kernel = ClosingKernelType::Box(radius);
	closing->SetKernel(kernel);
	closing->SetInput(itkImage);
	closing->SetForegroundValue(foregroundValue);
	closing->Update();
	auto closed = DicomProcessing::Copy<DicomImageType>(closing->GetOutput());

	filteredProjection = new DicomImage();
	filteredProjection->CopyInformation(projection);
	filteredProjection->SetItkDicomImage(closed);

	auto inversed = DicomProcessing::BinaryInverse(closed);
	auto tempFolder = std::string(helpers::GetAppDataTempPath());
	auto inversedImage = new DicomImage();
	inversedImage->SetItkDicomImage(inversed);
	inversedImage->SaveCompressed(
		(tempFolder + "\\Mandibular_TopSkullFilteredProjectionInversed.jpg")
		.c_str());

	auto keepN = KeepNType::New();
	keepN->SetInput(inversed);
	keepN->SetForegroundValue(foregroundValue);
	keepN->SetNumberOfObjects(1);
	keepN->SetBackgroundValue(0);
	keepN->SetAttribute("PhysicalSize");
	keepN->Update();
	auto afterKeep = keepN->GetOutput();
	if (afterKeep->GetPixel(itk::Index<2U> { 0, 0 } ) == foregroundValue)
	{
		// subtract outside
		auto subtract = SubtractType::New();
		subtract->SetInput1(inversed);
		subtract->SetInput2(afterKeep);
		subtract->Update();

		inversed = DicomProcessing::Copy<DicomImageType>(subtract->GetOutput());
		
		// subtract two middle "air" segments
		auto inversedCopy = DicomProcessing::Copy<DicomImageType>(inversed);
		// get two middle "air" segments image
		for (int i = 0; i < size[0] / 3; i++)
			for (int j = 0; j < size[1]; j++)
			{
				inversedCopy->SetPixel(itk::Index<2U> { i, j}, 0);
				inversedCopy->SetPixel(itk::Index<2U> { (int)size[0] - i, j}, 0);
			}

		keepN->SetInput(inversedCopy);
		keepN->SetNumberOfObjects(2);
		keepN->Update();
		auto midSeg = DicomProcessing::Copy<DicomImageType>(keepN->GetOutput());

		auto midSegImage = new DicomImage();
		midSegImage->SetItkDicomImage(midSeg);
		midSegImage->SaveCompressed(
			(tempFolder + "\\Mandibular_MidSegImage.jpg")
			.c_str());

		subtract->SetInput1(inversed);
		subtract->SetInput2(midSeg);
		subtract->Update();

		inversed = DicomProcessing::Copy<DicomImageType>(subtract->GetOutput());

		// subtract two "back" segments
		auto inversedCopy2 = DicomProcessing::Copy<DicomImageType>(inversed);
		// get two middle "air" segments image
		for (int i = 0; i < size[0]; i++)
			for (int j = 0; j < size[1] * 2 / 3; j++)
			{
				inversedCopy2->SetPixel(itk::Index<2U> { i, j}, 0);
			}

		keepN->SetInput(inversedCopy2);
		keepN->SetNumberOfObjects(2);
		keepN->Update();
		auto backSeg = DicomProcessing::Copy<DicomImageType>(keepN->GetOutput());

		auto backSegImage = new DicomImage();
		backSegImage->SetItkDicomImage(backSeg);
		backSegImage->SaveCompressed(
			(tempFolder + "\\Mandibular_BackSegImage.jpg")
			.c_str());

		subtract->SetInput1(inversed);
		subtract->SetInput2(backSeg);
		subtract->Update();
	
		// finalize
		inversed = DicomProcessing::Copy<DicomImageType>(subtract->GetOutput());
		keepN->SetInput(inversed);
		
		delete midSegImage;
		delete backSegImage;

		inversedImage->SetItkDicomImage(inversed);
		inversedImage->SaveCompressed(
			(tempFolder + "\\Mandibular_TopSkullFilteredProjectionAfterSubtractInversed.jpg")
			.c_str());
	}

	keepN->SetNumberOfObjects(2);
	keepN->Update();

	archSpacesProjection = new DicomImage();
	auto keeped = DicomProcessing::Copy<DicomImageType>(keepN->GetOutput());

	archSpacesProjection->SetItkDicomImage(keeped);

	auto cutsBackExt = GetCutPointsExternal(closed);
	auto cutsBackInt = GetCutPointsBackInternal(keeped);
	auto cutsFrontInt = GetCutPointsFrontInternal(keeped);

	std::cout << "[Zygomatic] Left cut points: \t" << cutsFrontInt[0][0] << "," << cutsFrontInt[0][1] << "\t;\t"
		<< cutsBackInt[0][0] << "," << cutsBackInt[0][1] << "\t;\t" << cutsBackExt[0][0] << "," << cutsBackExt[0][1] << std::endl;
	std::cout << "[Zygomatic] Right cut points: \t" << cutsFrontInt[1][0] << "," << cutsFrontInt[1][1] << "\t;\t"
		<< cutsBackInt[1][0] << "," << cutsBackInt[1][1] << "\t;\t" << cutsBackExt[1][0] << "," << cutsBackExt[1][1] << std::endl;

	auto leftYCut = cutsBackInt[0][0] > cutsFrontInt[0][0] ? cutsBackInt[0][0] : cutsFrontInt[0][0];
	auto rightYCut = cutsBackInt[1][0] < cutsFrontInt[1][0] ? cutsBackInt[1][0] : cutsFrontInt[1][0];
	auto leftZCut = cutsFrontInt[0][1];
	auto rightZCut = cutsFrontInt[1][1];
	auto leftLineSlop = (double)(cutsBackInt[0][1] - cutsBackExt[0][1]) / (cutsBackInt[0][0] - cutsBackExt[0][0]);
	auto rightLineSlop = (double)(cutsBackInt[1][1] - cutsBackExt[1][1]) / (cutsBackInt[1][0] - cutsBackExt[1][0]);
	auto leftLineComp = cutsBackInt[0][1] - (double)(leftLineSlop * cutsBackInt[0][0]);
	auto rightLineComp = cutsBackInt[1][1] - (double)(rightLineSlop * cutsBackInt[1][0]);
	
	auto deg1 = std::atan(leftLineSlop) / itk::Math::pi * 180;
	auto deg2 = std::atan(rightLineSlop) / itk::Math::pi * 180;
	std::cout << "[Zygomatic] Left line angle: " << deg1 << std::endl;
	std::cout << "[Zygomatic] Right line angle: " << deg2 << std::endl;
	rotateAngle = (deg1 + deg2) / 2;
	std::cout << "[Zygomatic] Angles diff: " << rotateAngle << std::endl;

	// select lines
	for (int i = 0; i < size[0]; i++)
	{
		auto leftVal = (int)(leftLineSlop * i + leftLineComp);
		auto rightVal = (int)(rightLineSlop * i + rightLineComp);
		if (leftVal >= 0 && leftVal <= size[1])
			closed->SetPixel(itk::Index<2U> { i, leftVal }, 25000);

		if (rightVal >= 0 && rightVal <= size[1])
			closed->SetPixel(itk::Index<2U> { i, rightVal }, 25000);

		closed->SetPixel(itk::Index<2U> { leftYCut, i }, 20000);
		closed->SetPixel(itk::Index<2U> { rightYCut, i }, 20000);
		closed->SetPixel(itk::Index<2U> { i, cutsFrontInt[0][1] }, 20000);
		closed->SetPixel(itk::Index<2U> { i, cutsFrontInt[1][1] }, 20000);
	}

	delete inversedImage;

	archImage = new DicomSeries();
	auto itkArch = DicomSeriesType::New();
	itkArch->SetRegions(image->GetItkImageData()->GetLargestPossibleRegion());
	itkArch->SetSpacing(image->GetItkImageData()->GetSpacing());
	itkArch->Allocate();
	itkArch->FillBuffer(0);
	using ConstIteratorType = itk::ImageRegionConstIterator<DicomSeriesType>;
	using IteratorType = itk::ImageRegionIterator<DicomSeriesType>;
	auto constIt = ConstIteratorType(image->GetItkImageData(), image->GetItkImageData()->GetLargestPossibleRegion());
	auto it = IteratorType(itkArch, itkArch->GetLargestPossibleRegion());
	constIt.GoToBegin(); it.GoToBegin();
	while (!constIt.IsAtEnd())
	{
		if (constIt.Get() == foregroundValue)
		{
			auto in = constIt.GetIndex();
			
			if (in[2] < nasion[2])
			{
				if (in[0] < size[0] / 2)
				{
					auto lineVal = leftLineSlop * in[0] + leftLineComp;
					if (in[1] <= lineVal && in[0] < leftYCut && in[1] >= leftZCut + 3)
						it.Set(foregroundValue);
					else
						it.Set(1000);
				}
				else
				{
					auto lineVal = rightLineSlop * in[0] + rightLineComp;
					if (in[1] <= lineVal && in[0] > rightYCut && in[1] >= rightZCut + 3)
						it.Set(foregroundValue);
					else
						it.Set(1000);
				}
			}
			else
				it.Set(1000);
		}

		++constIt;
		++it;
	}
	archImage->SetItkImageData(itkArch);

	archsProjection = CreateTopProjection(archImage);
	//archsProjection->SaveCompressed((tempFolder + "\\Mandibular_ArchsProjection.jpg").c_str());

	return archImage;
}

std::vector<DicomImageType::IndexType>
ZygomaticArchPointsExtractor::GetCutPointsExternal(
	DicomImageType::Pointer input)
{
	std::vector<DicomImageType::IndexType> points;

	using ConstLineIterator = itk::ImageLinearConstIteratorWithIndex<DicomImageType>;
	auto it = ConstLineIterator(input, input->GetLargestPossibleRegion());
	it.GoToReverseBegin();

	auto middleX = input->GetLargestPossibleRegion().GetSize()[0] / 2;
	DicomImageType::IndexType leftPt;
	unsigned int tempMin = UINT_MAX;
	DicomImageType::IndexType prevLeft;
	bool leftFound = false;
	while (!it.IsAtReverseEnd())
	{
		it.GoToBeginOfLine();
		while (!it.IsAtEndOfLine())
		{
			if (it.Get() == foregroundValue)
			{
				auto in = it.GetIndex();
				if (in[0] < middleX && !leftFound)
				{
					if (in[0] < tempMin)
					{
						tempMin = in[0];
					}
					else if (tempMin != UINT_MAX
						&& std::abs(tempMin - in[0]) > 3
						&& prevLeft[0] > in[0])
					{
						leftFound = true;
						leftPt = prevLeft;
					}
					prevLeft = in;
				}
				break;
			}
			++it;
		}

		if (leftFound) break;

		it.PreviousLine();
	}
	points.push_back(leftPt);

	it.GoToReverseBegin();
	DicomImageType::IndexType rightPt;
	unsigned int tempMax = 0;
	DicomImageType::IndexType prevRight;
	bool rightFound = false;
	while (!it.IsAtReverseEnd())
	{
		it.GoToReverseBeginOfLine();
		while (!it.IsAtReverseEndOfLine())
		{
			if (it.Get() == foregroundValue)
			{
				auto in = it.GetIndex();
				if (in[0] > middleX && !rightFound)
				{
					if (in[0] > tempMax)
					{
						tempMax = in[0];
					}
					else if (tempMax != 0
						&& std::abs(tempMax - in[0]) > 3
						&& prevRight[0] < in[0])
					{
						rightFound = true;
						rightPt = prevRight;
					}
					prevRight = in;
				}
				break;
			}
			--it;
		}

		if (rightFound) break;

		it.PreviousLine();
	}


	points.push_back(rightPt);
	return points;
}

std::vector<DicomImageType::IndexType>
ZygomaticArchPointsExtractor::GetCutPointsBackInternal(
	DicomImageType::Pointer image)
{
	std::vector<DicomImageType::IndexType> points;

	using ConstLineIterator = itk::ImageLinearConstIteratorWithIndex<DicomImageType>;
	auto it = ConstLineIterator(image, image->GetLargestPossibleRegion());
	it.GoToReverseBegin();

	auto middleX = image->GetLargestPossibleRegion().GetSize()[0] / 2;
	DicomImageType::IndexType leftPt;
	DicomImageType::IndexType rightPt;
	bool leftFound = false, rightFound = false;
	while (!it.IsAtReverseEnd())
	{
		it.GoToBeginOfLine();
		while (!it.IsAtEndOfLine())
		{
			if (it.Get() == foregroundValue)
			{
				if (it.GetIndex()[0] > middleX && !rightFound)
				{
					rightFound = true;
					rightPt = it.GetIndex();
					break;
				}
			}
			++it;
		}

		it.GoToReverseBeginOfLine();
		while (!it.IsAtReverseEndOfLine())
		{
			if (it.Get() == foregroundValue)
			{
				if (it.GetIndex()[0] < middleX && !leftFound)
				{
					leftFound = true;
					leftPt = it.GetIndex();
					break;
				}
			}
			--it;
		}

		it.PreviousLine();
	}
	points.push_back(leftPt);
	points.push_back(rightPt);
	return points;
}

std::vector<DicomImageType::IndexType>
ZygomaticArchPointsExtractor::GetCutPointsFrontInternal(
	DicomImageType::Pointer image)
{
	std::vector<DicomImageType::IndexType> points;

	using ConstLineIterator = itk::ImageLinearConstIteratorWithIndex<DicomImageType>;
	auto it = ConstLineIterator(image, image->GetLargestPossibleRegion());
	it.GoToBegin();

	auto middleX = image->GetLargestPossibleRegion().GetSize()[0] / 2;
	DicomImageType::IndexType leftPt;
	DicomImageType::IndexType rightPt;
	bool leftFound = false, rightFound = false;
	while (!it.IsAtEnd())
	{
		it.GoToBeginOfLine();
		while (!it.IsAtEndOfLine())
		{
			if (it.Get() == foregroundValue)
			{
				if (it.GetIndex()[0] < middleX && !leftFound)
				{
					leftFound = true;
					leftPt = it.GetIndex();
					break;
				}
			}
			++it;
		}

		it.GoToReverseBeginOfLine();
		while (!it.IsAtReverseEndOfLine())
		{
			if (it.Get() == foregroundValue)
			{
				if (it.GetIndex()[0] > middleX && !rightFound)
				{
					rightFound = true;
					rightPt = it.GetIndex();
					break;
				}
			}
			--it;
		}

		it.NextLine();
	}
	points.push_back(leftPt);
	points.push_back(rightPt);
	return points;
}

DicomImage* ZygomaticArchPointsExtractor::GetRotatedImage(DicomImage* input)
{
	auto itkImage = input->GetItkDicomImage();
	std::cout << "[Zygomatic] Creating rotated projection..." << std::endl;
	using FilterType = itk::ResampleImageFilter<DicomImageType, DicomImageType>;
	auto filter = FilterType::New();
	
	using TransformType = itk::AffineTransform<double, 2U>;
	auto transform = TransformType::New();

	// MATRIX TRANSFORM
	auto radDeg = rotateAngle / 180 * itk::Math::pi;
	rotateMatrix[0][0] = std::cos(radDeg);
	rotateMatrix[0][1] = -std::sin(radDeg);
	rotateMatrix[1][0] = std::sin(radDeg);
	rotateMatrix[1][1] = std::cos(radDeg);

	std::setprecision(6);
	std::cout << "[Zygomatic] 2D Matrix: " << rotateMatrix(0, 0) << " , " << rotateMatrix(0, 1) << std::endl;
	std::cout << "[Zygomatic] 2D Matrix: " << rotateMatrix(1, 0) << " , " << rotateMatrix(1, 1) << std::endl;

	transform->SetMatrix(rotateMatrix);
	filter->SetTransform(transform);

	using InterpolatorType = itk::NearestNeighborInterpolateImageFunction<DicomImageType, double>;
	auto interpolator = InterpolatorType::New();
	filter->SetInterpolator(interpolator);
	filter->SetDefaultPixelValue(0);
	filter->SetOutputSpacing(itkImage->GetSpacing());
	filter->SetOutputOrigin(itkImage->GetOrigin());
	filter->SetOutputDirection(itkImage->GetDirection());
	filter->SetSize(itkImage->GetLargestPossibleRegion().GetSize());

	filter->SetInput(itkImage);
	filter->Update();

	auto newImage = new DicomImage();
	newImage->SetItkDicomImage(filter->GetOutput());
	return newImage;
}

DicomSeries* ZygomaticArchPointsExtractor::GetRotatedImage3D(DicomSeries* input)
{
	std::cout << "[Zygomatic] Creating rotated 3D projection..." << std::endl;
	auto itkImage = input->GetItkImageData();
	using FilterType = itk::ResampleImageFilter<DicomSeriesType, DicomSeriesType>;
	auto filter = FilterType::New();

	using TransformType = itk::AffineTransform<double, 3U>;
	auto transform = TransformType::New();

	// MATRIX TRANSFORM
	auto radDeg = rotateAngle / 180 * itk::Math::pi;
	rotateMatrix3D[0][0] = std::cos(radDeg);
	rotateMatrix3D[0][1] = -std::sin(radDeg);
	rotateMatrix3D[1][0] = std::sin(radDeg);
	rotateMatrix3D[1][1] = std::cos(radDeg);
	rotateMatrix3D[2][2] = 1;

	std::cout << "[Zygomatic] 3D Matrix: " << rotateMatrix3D(0, 0) << " , " << rotateMatrix3D(0, 1) << " , " << rotateMatrix3D(0, 2) << std::endl;
	std::cout << "[Zygomatic] 3D Matrix: " << rotateMatrix3D(1, 0) << " , " << rotateMatrix3D(1, 1) << " , " << rotateMatrix3D(1, 2) << std::endl;
	std::cout << "[Zygomatic] 3D Matrix: " << rotateMatrix3D(2, 0) << " , " << rotateMatrix3D(2, 1) << " , " << rotateMatrix3D(2, 2) << std::endl;

	transform->SetMatrix(rotateMatrix3D);
	filter->SetTransform(transform);

	using InterpolatorType = itk::NearestNeighborInterpolateImageFunction<DicomSeriesType, double>;
	auto interpolator = InterpolatorType::New();
	filter->SetInterpolator(interpolator);
	filter->SetDefaultPixelValue(0);
	filter->SetOutputSpacing(itkImage->GetSpacing());
	filter->SetOutputOrigin(itkImage->GetOrigin());
	filter->SetOutputDirection(itkImage->GetDirection());
	filter->SetSize(itkImage->GetLargestPossibleRegion().GetSize());

	filter->SetInput(itkImage);
	filter->Update();

	auto newImage = new DicomSeries();
	newImage->SetItkImageData(filter->GetOutput());
	return newImage;
}

ZygomaticArchPointsExtractor::~ZygomaticArchPointsExtractor()
{
	if (image != nullptr)
		delete image;
	if (archImage != nullptr)
		delete archImage;
}
