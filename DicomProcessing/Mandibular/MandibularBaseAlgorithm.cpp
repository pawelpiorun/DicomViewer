#include "MandibularBaseAlgorithm.h"
#include "helpers.h"

//#define LOGITERATIONSINFO

MandibularBaseAlgorithm::MandibularBaseAlgorithm()
{
	progressAccumulator = itk::ProgressAccumulator::New();

	medianFilter = MedianFilterType::New();
	medianFilter->SetRadius(1);
	medianFilter->SetReleaseDataFlag(true);

	teethThreshold = ThresholdType::New();
	teethThreshold->SetLowerThreshold(2300);
	teethThreshold->SetUpperThreshold(2600);
	teethThreshold->SetInsideValue(_selectionValue);
	teethThreshold->SetOutsideValue(0);
	teethThreshold->SetReleaseDataFlag(true);

	teethDilate = DilateType::New();
	DilateKernelType::RadiusType dilateRadius;
	dilateRadius.Fill(2);
	auto dilateKernel = DilateKernelType::Box(dilateRadius);
	teethDilate->SetForegroundValue(_selectionValue);
	teethDilate->SetBackgroundValue(0);
	teethDilate->SetKernel(dilateKernel);
	teethDilate->SetInput(teethThreshold->GetOutput());
	teethDilate->SetReleaseDataFlag(true);

	keepNFilter = KeepNFilterType::New();
	keepNFilter->SetInput(teethDilate->GetOutput());
	keepNFilter->SetForegroundValue(_selectionValue);
	keepNFilter->SetBackgroundValue(0);
	keepNFilter->SetNumberOfObjects(1);
	keepNFilter->SetReleaseDataFlag(true);

	generalThreshold = ThresholdType::New();
	generalThreshold->SetLowerThreshold(400);
	generalThreshold->SetInsideValue(_selectionValue);
	generalThreshold->SetOutsideValue(SHRT_MIN);
	//generalThreshold->SetReleaseDataFlag(true);

	subtract = SubtractFilterType::New();
	subtract->SetInput1(generalThreshold->GetOutput());
	//subtract->SetInput2(keepNFilter->GetOutput());
	//subtract->SetReleaseDataFlag(true);

	connectedThreshold = ConnectedThresholdType::New();
	connectedThreshold->SetInput(subtract->GetOutput());
	connectedThreshold->SetLower(1000);
	connectedThreshold->SetReplaceValue(SHRT_MAX);
	//connectedThreshold->SetReleaseDataFlag(true);

	mandibulaSubtract = SubtractFilterType::New();

	//labelStatistics = LabelStatisticsFilter::New();
	//labelStatistics->SetInput(subtract->GetOutput());
	//labelStatistics->SetLabelInput(subtract->GetOutput());

	nasionPointExtractor = NasionPointExtractor::New();

	boneToCondyles = MandibularBoneToCondylesType::New();
	boneToCondyles->SetInput(connectedThreshold->GetOutput());

	

	//progressAccumulator->RegisterInternalFilter(medianFilter, 0.2f);
	progressAccumulator->RegisterInternalFilter(teethThreshold, 0.05f);
	progressAccumulator->RegisterInternalFilter(teethDilate, 0.35f);
	progressAccumulator->RegisterInternalFilter(generalThreshold, 0.05f);
	progressAccumulator->RegisterInternalFilter(subtract, 0.05f);
	progressAccumulator->RegisterInternalFilter(boneToCondyles, 0.5f);
	//progressAccumulator->RegisterInternalFilter(labelStatistics, 0.1f);
}

std::vector<MandibularBaseAlgorithm::ImageSource<MandibularBaseAlgorithm::ImageType>::Pointer>
MandibularBaseAlgorithm::GetProcessObjects()
{
	return std::vector<ImageSource<ImageType>::Pointer>
	{
		teethThreshold,
		teethDilate,
		keepNFilter,
		generalThreshold,
		subtract,
		connectedThreshold,
		mandibulaSubtract,
		boneToCondyles
	};
}

int MandibularBaseAlgorithm::Run(DicomSeries* dicomSeries, bool keepOriginalImage)
{
	this->input = dicomSeries;
	this->SetInput(dicomSeries->GetItkImageData());
	this->Update();
	if (!success) return 1;

	if (!keepOriginalImage)
	{
		dicomSeries->SetItkImageData(this->GetOutput());
	}

	if (keepOriginalImage)
	{
		auto output = this->GetOutput();
		auto imageData = dicomSeries->GetItkImageData();

		using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
		using IteratorType = itk::ImageRegionIterator<ImageType>;

		auto inputRegion = output->GetLargestPossibleRegion();
		auto outputRegion = imageData->GetLargestPossibleRegion();
		auto srcIt = ConstIteratorType(output, inputRegion);
		auto dstIt = IteratorType(imageData, outputRegion);
		srcIt.GoToBegin();
		dstIt.GoToBegin();

		int count = 0;
		while (!srcIt.IsAtEnd() && !dstIt.IsAtEnd())
		{
			auto val = srcIt.Value();
			if (val == _selectionValue)
			{
				dstIt.Set(_selectionValue);
				count++;
			}
			++srcIt;
			++dstIt;
		}

		std::cout << std::endl << "[Mandibular condyle] Selected points: " << count << std::endl;
	}

	return 0;
}

void MandibularBaseAlgorithm::GenerateData()
{
	success = true;
	auto localInput = DicomSeriesType::New();
	localInput->Graft(const_cast<DicomSeriesType*>(this->GetInput()));
	progressAccumulator->SetMiniPipelineFilter(this);

	medianFilter->SetInput(localInput);
	medianFilter->Update();
	auto medianInput = DicomProcessing::Copy<DicomSeriesType>(medianFilter->GetOutput());
	generalThreshold->SetInput(medianInput);
	teethThreshold->SetInput(medianInput);

	auto lastPipeline = GetLastPipelineObject();
	lastPipeline->SetReleaseDataFlag(false);
	auto objects = GetProcessObjects();
	auto subIt = std::find(objects.begin(), objects.end(), subtract);
	auto connIt = std::find(objects.begin(), objects.end(), subtract);
	auto msubIt = std::find(objects.begin(), objects.end(), mandibulaSubtract);
	auto subIndex = std::distance(objects.begin(), subIt);
	auto connIndex = std::distance(objects.begin(), connIt);
	auto msubIndex = std::distance(objects.begin(), msubIt);
	auto lastIt = std::find(objects.begin(), objects.end(), lastPipeline);
	auto lastIndex = std::distance(objects.begin(), lastIt);

	if (lastIndex < subIndex)
	{
		try
		{
			lastPipeline->Update();
		}
		catch (itk::ExceptionObject x)
		{
			std::cout << x.GetDescription() << std::endl;
			success = false;
		}
		if (success)
			this->GraftOutput(lastPipeline->GetOutput());
		else
			this->GraftOutput(localInput);
		return;
	}

	// lastpipeline > subtract

	try
	{
		keepNFilter->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
		success = false;
		this->GraftOutput(localInput);
		return;
	}

	auto subInput = DicomProcessing::Copy<DicomSeriesType>(keepNFilter->GetOutput());
	subtract->SetInput2(subInput);
	
	try
	{
		subtract->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
		success = false;
		this->GraftOutput(localInput);
		return;
	}

	// looking for mandibular points
	auto meanTeethZ = GetMeanZCoord(subInput);
	//RemovePointsAbove(output, meanZ);
	auto boundingBoxXYZ = DicomProcessing::GetSelectionBoundingBoxXYZ(subInput, _selectionValue);
	auto pointsBelow = GetPointsBelowZ(subtract->GetOutput(), boundingBoxXYZ, meanTeethZ);
	auto meanPointsBelowZ = GetMeanZCoord(pointsBelow);

	for (int i = 0; i < pointsBelow.size(); i++)
	{
		auto point = pointsBelow[i];
		if (point[2] < meanPointsBelowZ)
			connectedThreshold->AddSeed(point);
	}


	// nasion point
	nasionPointExtractor->SetImage(generalThreshold->GetOutput());
	nasionPointExtractor->SetZThreshold(boundingBoxXYZ[1][2] + 20); // max z 
	nasionPointExtractor->Compute();
	auto nasionIndex = nasionPointExtractor->GetNasionPoint();
	for (unsigned int i = 0; i < nasionIndex.size(); i++) nasionPoint[i] = nasionIndex[i];

	upperSkullProjection = nasionPointExtractor->GetUpperSkullImage();
	auto tempFolder = std::string(helpers::GetAppDataTempPath());
	upperSkullProjection->SaveCompressed((tempFolder + "\\" + upperSkullProjection->GetFileName() + ".jpg").c_str());

	if (lastIndex == subIndex)
	{
		this->GraftOutput(subtract->GetOutput());
		return;
	}

	int iter = 1;
	do
	{
		try
		{
			connectedThreshold->Update();
		}
		catch (itk::ExceptionObject x)
		{
			std::cout << x.GetDescription() << std::endl;
			success = false;
			return;
		}

		auto subVoxelCount = DicomProcessing::GetVoxelCount(subtract->GetOutput(), SHRT_MAX);
		auto connVoxelCount = DicomProcessing::GetVoxelCount(connectedThreshold->GetOutput(), SHRT_MAX);
		success = connVoxelCount <= 0.75 * subVoxelCount;

#ifdef LOGITERATIONSINFO
		std::cout << "Iter: " << iter << " Sub: " << subVoxelCount << "\t (75%: " << 0.75 * subVoxelCount << ")"
			<< " Con: " << connVoxelCount;
#endif // LOGITERATIONSINFO


		if (!success)
		{
			auto threshold = generalThreshold->GetLowerThreshold();
			generalThreshold->SetLowerThreshold(threshold + 10);

#ifdef LOGITERATIONSINFO
			std::cout << " Next th: " << threshold + 10 << std::endl;
#endif // LOGITERATIONSINFO

			iter++;
			generalThreshold->UpdateProgress(0.0);
			subtract->UpdateProgress(0.0);
			connectedThreshold->UpdateProgress(0.0);
		}
#ifdef LOGITERATIONSINFO
		else
			std::cout << std::endl;
#endif

	} while (!success && iter < 100);

	if (!success)
	{
		errorMessage = "Failed to extract mandibular bone.";
		std::cout << errorMessage << std::endl;
		return;
	}

	if (lastIndex > connIndex)
	{
		subtract->SetReleaseDataFlag(true);
		mandibulaSubtract->SetInput1(subtract->GetOutput());
		mandibulaSubtract->SetInput2(connectedThreshold->GetOutput());
		
		try
		{
			connectedThreshold->SetReleaseDataFlag(true);
			mandibulaSubtract->Update();
			if (lastIndex != msubIndex)
				lastPipeline->Update();
		}
		catch (itk::ExceptionObject x)
		{
			std::cout << x.GetDescription() << std::endl;
			success = false;
			return;
		}

		// todo: extract zygomatic archs points
		auto zygomaticExtractor = new ZygomaticArchPointsExtractor();
		zygomaticExtractor->SetImage(mandibulaSubtract->GetOutput());
		zygomaticExtractor->SetNasionPoint(nasionPoint);
		zygomaticExtractor->Compute();

		topCutProjection = zygomaticExtractor->GetProjection();
		auto tempFolder = std::string(helpers::GetAppDataTempPath());
		topCutProjection->SetFileName("Mandibular_TopSkullProjeciton");
		topCutProjection->SaveCompressed((tempFolder + "\\Mandibular_TopSkullProjection.jpg").c_str());

		topCutFilteredProjection = zygomaticExtractor->GetFilteredProjection();
		topCutFilteredProjection->SetFileName("Mandibular_TopSkullFilteredProjeciton");
		topCutFilteredProjection->SaveCompressed((tempFolder + "\\Mandibular_TopSkullFilteredProjection.jpg").c_str());

		archSpacesProjection = zygomaticExtractor->GetArchSpacesProjection();
		archSpacesProjection->SetFileName("Mandibular_TopProjectionArchSpaces");
		archSpacesProjection->SaveCompressed((tempFolder + "\\Mandibular_TopProjectionArchSpaces.jpg").c_str());

		archsProjection = zygomaticExtractor->GetArchProjection();
		archsProjection->SetFileName("Mandibular_TopProjectionArchs");
		archsProjection->SaveCompressed((tempFolder + "\\Mandibular_TopProjectionArchs.jpg").c_str());

		auto zygPts = zygomaticExtractor->GetZygomaticPoints();
		leftZygomaticPoint = std::vector<int>{ (int) zygPts[0][0], (int) zygPts[0][1], (int) zygPts[0][2] };
		rightZygomaticPoint = std::vector<int>{ (int) zygPts[1][0], (int) zygPts[1][1], (int) zygPts[1][2] };

		if (lastIndex > msubIndex)
		{
			CalculateCondylesInfo();
			leftLinesImage = boneToCondyles->GetLeftLinesImage();
			rightLinesImage = boneToCondyles->GetRightLinesImage();
			leftLinesImage->SaveCompressed((tempFolder + "\\" + leftLinesImage->GetFileName() + ".jpg").c_str());
			rightLinesImage->SaveCompressed((tempFolder + "\\" + rightLinesImage->GetFileName() + ".jpg").c_str());

		}		
		
		if (endWithZygomaticArchs)
		{
			this->GraftOutput(zygomaticExtractor->GetArchImage()->GetItkImageData());
			delete zygomaticExtractor;
			return;
		}
		else
			delete zygomaticExtractor;

	}

	this->GraftOutput(lastPipeline->GetOutput());
}

void MandibularBaseAlgorithm::CalculateCondylesInfo()
{
	// condyles
	auto left = boneToCondyles->GetLeftCondyleCenter();
	auto right = boneToCondyles->GetRightCondyleCenter();
	for (unsigned int i = 0; i < left.size(); i++)
	{
		leftCondylePos[i] = left[i];
		rightCondylePos[i] = right[i];
		condylesPosDiff[i] = right[i] - left[i];
		left2NasionDiff[i] = nasionPoint[i] - left[i];
		right2NasionDiff[i] = nasionPoint[i] - right[i];
	}

	condylesDistancePx = std::sqrt(
		std::pow(right[0] - left[0], 2) +
		std::pow(right[1] - left[1], 2) +
		std::pow(right[2] - left[2], 2));

	auto localInput = const_cast<DicomSeriesType*>(this->GetInput());
	auto spacing = localInput->GetSpacing();

	std::vector<double> diffInMm = std::vector<double>(3);
	diffInMm[0] = condylesPosDiff[0] * spacing[0];
	diffInMm[1] = condylesPosDiff[1] * spacing[1];
	diffInMm[2] = condylesPosDiff[2] * spacing[2];
	condylesDistance = std::sqrt(
		std::pow(diffInMm[0], 2) +
		std::pow(diffInMm[1], 2) +
		std::pow(diffInMm[2], 2));

	std::cout << "[Condyles] Left: " << left[0] << ", " << left[1] << ", " << left[2] << std::endl;
	std::cout << "[Condyles] Right: " << right[0] << ", " << right[1] << ", " << right[2] << std::endl;
	std::cout << "[Condyles] Diff: " << condylesPosDiff[0] << ", " << condylesPosDiff[1] << ", " << condylesPosDiff[2] << std::endl;
	std::cout << "[Condyles] Distance in px: " << condylesDistancePx << std::endl;
	std::cout << "[Condyles] Distance in mm: " << condylesDistance << std::endl;

	// with nasion
	left2NasionDistancePx = std::sqrt(
		std::pow(left2NasionDiff[0], 2) +
		std::pow(left2NasionDiff[1], 2) +
		std::pow(left2NasionDiff[2], 2));
	right2NasionDistancePx = std::sqrt(
		std::pow(right2NasionDiff[0], 2) +
		std::pow(right2NasionDiff[1], 2) +
		std::pow(right2NasionDiff[2], 2));
	std::vector<double> left2NasionDiffInMM = std::vector<double>(3);
	left2NasionDiffInMM[0] = left2NasionDiff[0] * spacing[0];
	left2NasionDiffInMM[1] = left2NasionDiff[1] * spacing[1];
	left2NasionDiffInMM[2] = left2NasionDiff[2] * spacing[2];
	left2NasionDistance = std::sqrt(
		std::pow(left2NasionDiffInMM[0], 2) +
		std::pow(left2NasionDiffInMM[1], 2) +
		std::pow(left2NasionDiffInMM[2], 2));
	std::vector<double> right2NasionDiffInMM = std::vector<double>(3);
	right2NasionDiffInMM[0] = right2NasionDiff[0] * spacing[0];
	right2NasionDiffInMM[1] = right2NasionDiff[1] * spacing[1];
	right2NasionDiffInMM[2] = right2NasionDiff[2] * spacing[2];
	right2NasionDistance = std::sqrt(
		std::pow(right2NasionDiffInMM[0], 2) +
		std::pow(right2NasionDiffInMM[1], 2) +
		std::pow(right2NasionDiffInMM[2], 2));

	std::cout << "[Condyles] Distance nasion-right in mm: " << right2NasionDistance << std::endl;
	std::cout << "[Condyles] Distance nasion-left in mm: " << left2NasionDistance << std::endl;

	// zygomatic with nasion
	center[0] = (nasionPoint[0] + leftZygomaticPoint[0] + rightZygomaticPoint[0]) / 3;
	center[1] = (nasionPoint[1] + leftZygomaticPoint[1] + rightZygomaticPoint[1]) / 3;
	center[2] = (nasionPoint[2] + leftZygomaticPoint[2] + rightZygomaticPoint[2]) / 3;
	left2CenterDiff[0] = center[0] - left[0];
	left2CenterDiff[1] = center[1] - left[1];
	left2CenterDiff[2] = center[2] - left[2];
	right2CenterDiff[0] = center[0] - right[0];
	right2CenterDiff[1] = center[1] - right[1];
	right2CenterDiff[2] = center[2] - right[2];

	std::vector<double> left2CenterDistInMM = std::vector<double>(3);
	left2CenterDistInMM[0] = left2CenterDiff[0] * spacing[0];
	left2CenterDistInMM[1] = left2CenterDiff[1] * spacing[1];
	left2CenterDistInMM[2] = left2CenterDiff[2] * spacing[2];
	left2CenterDistance = std::sqrt(
		std::pow(left2CenterDistInMM[0], 2) +
		std::pow(left2CenterDistInMM[1], 2) +
		std::pow(left2CenterDistInMM[2], 2));
	std::vector<double> right2CenterDistInMM = std::vector<double>(3);
	right2CenterDistInMM[0] = right2CenterDiff[0] * spacing[0];
	right2CenterDistInMM[1] = right2CenterDiff[1] * spacing[1];
	right2CenterDistInMM[2] = right2CenterDiff[2] * spacing[2];
	right2CenterDistance = std::sqrt(
		std::pow(right2CenterDistInMM[0], 2) +
		std::pow(right2CenterDistInMM[1], 2) +
		std::pow(right2CenterDistInMM[2], 2));

	this->spacing[0] = spacing[0];
	this->spacing[1] = spacing[1];
	this->spacing[2] = spacing[2];

	auto origin = localInput->GetOrigin();
	this->origin[0] = origin[0];
	this->origin[1] = origin[1];
	this->origin[2] = origin[2];

	// get new origin in physical coords
	auto zygVec = itk::Vector<double>();
	zygVec[0] = (double)(rightZygomaticPoint[0] - leftZygomaticPoint[0]) * spacing[0];
	zygVec[1] = (double)(rightZygomaticPoint[1] - leftZygomaticPoint[1]) * spacing[1];
	zygVec[2] = (double)(rightZygomaticPoint[2] - leftZygomaticPoint[2]) * spacing[2];
	
	auto zyg2NVec = itk::Vector<double>();
	zyg2NVec[0] = (double)(nasionPoint[0] - leftZygomaticPoint[0]) * spacing[0];
	zyg2NVec[1] = (double)(nasionPoint[1] - leftZygomaticPoint[1]) * spacing[1];
	zyg2NVec[2] = (double)(nasionPoint[2] - leftZygomaticPoint[2]) * spacing[2];

	auto k = zyg2NVec * zygVec / (zygVec * zygVec);

	std::vector<double> newOrigin = std::vector<double>(3);
	newOrigin[0] = leftZygomaticPoint[0] * spacing[0] + origin[0] + k * zygVec[0];
	newOrigin[1] = leftZygomaticPoint[1] * spacing[1] + origin[1] + k * zygVec[1];
	newOrigin[2] = leftZygomaticPoint[2] * spacing[2] + origin[2] + k * zygVec[2];

	std::cout << "[Condyles] ZygVec: " << zygVec[0] << "," << zygVec[1] << "," << zygVec[2] << std::endl;
	std::cout << "[Condyles] Ztg2NVec: " << zyg2NVec[0] << "," << zyg2NVec[1] << "," << zyg2NVec[2] << std::endl;
	std::cout << "[Condyles] k: " << k << std::endl;
	std::cout << "[Condyles] Left zyg real: " << leftZygomaticPoint[0] * spacing[0] + origin[0] << std::endl;
	std::cout << "[Condyles] Left zyg real: " << leftZygomaticPoint[1] * spacing[1] + origin[1] << std::endl;
	std::cout << "[Condyles] Left zyg real: " << leftZygomaticPoint[2] * spacing[2] + origin[2] << std::endl;


	std::cout << "[Condyles] New origin: " << newOrigin[0]
		<< "," << newOrigin[1] << "," << newOrigin[2] << std::endl;

	auto xAxis = itk::Vector<double>();
	xAxis[0] = leftZygomaticPoint[0] * spacing[0] + origin[0] - newOrigin[0];
	xAxis[1] = leftZygomaticPoint[1] * spacing[1] + origin[1] - newOrigin[1];
	xAxis[2] = leftZygomaticPoint[2] * spacing[2] + origin[2] - newOrigin[2];
	xAxis.Normalize();

	auto zAxis = itk::Vector<double>();
	zAxis[0] = nasionPoint[0] * spacing[0] + origin[0] - newOrigin[0];
	zAxis[1] = nasionPoint[1] * spacing[1] + origin[1] - newOrigin[1];
	zAxis[2] = nasionPoint[2] * spacing[2] + origin[2] - newOrigin[2];
	zAxis.Normalize();

	auto yAxis = itk::CrossProduct(zAxis, xAxis);
	yAxis.Normalize();

	std::cout << "[Condyles] X axis: " << xAxis[0] << "," << xAxis[1] << "," << xAxis[2] << std::endl;
	std::cout << "[Condyles] Y axis: " << yAxis[0] << "," << yAxis[1] << "," << yAxis[2] << std::endl;
	std::cout << "[Condyles] Z axis: " << zAxis[0] << "," << zAxis[1] << "," << zAxis[2] << std::endl;

	// transform origin
	double* srcPoints = new double[9]
	{
		newOrigin[0] + xAxis[0], newOrigin[1] + xAxis[1], newOrigin[2] + xAxis[2],
		newOrigin[0] + yAxis[0], newOrigin[1] + yAxis[1], newOrigin[2] + yAxis[2],
		newOrigin[0] + zAxis[0], newOrigin[1] + zAxis[1], newOrigin[2] + zAxis[2]
	};
	double* srcCenter = new double[3] { newOrigin[0], newOrigin[1], newOrigin[2] };
	double* trgPoints = new double[9]{ 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	double* trgCenter = new double[3]{ 0, 0, 0 };

	auto transform = DicomProcessing::CalculateTransformationMatrix(srcPoints, srcCenter, trgPoints, trgCenter);

	// get condyles coords in new physical world
	auto leftCondyleReal = std::vector<double>
	{
		leftCondylePos[0] * spacing[0] + origin[0],
		leftCondylePos[1] * spacing[1] + origin[1],
		leftCondylePos[2] * spacing[2] + origin[2]
	};
	leftCondyleFinal = std::vector<double>
	{
		leftCondyleReal[0] * transform[0] + leftCondyleReal[1] * transform[1] + leftCondyleReal[2] * transform[2] + transform[3],
		leftCondyleReal[0] * transform[4] + leftCondyleReal[1] * transform[5] + leftCondyleReal[2] * transform[6] + transform[7],
		leftCondyleReal[0] * transform[8] + leftCondyleReal[1] * transform[9] + leftCondyleReal[2] * transform[10] + transform[11]
	};
	auto rightCondyleReal = std::vector<double>
	{
		rightCondylePos[0] * spacing[0] + origin[0],
		rightCondylePos[1] * spacing[1] + origin[1],
		rightCondylePos[2] * spacing[2] + origin[2]
	};
	rightCondyleFinal = std::vector<double>
	{
		rightCondyleReal[0] * transform[0] + rightCondyleReal[1] * transform[1] + rightCondyleReal[2] * transform[2] + transform[3],
		rightCondyleReal[0] * transform[4] + rightCondyleReal[1] * transform[5] + rightCondyleReal[2] * transform[6] + transform[7],
		rightCondyleReal[0] * transform[8] + rightCondyleReal[1] * transform[9] + rightCondyleReal[2] * transform[10] + transform[11]
	};

	std::cout << "[Condyles] Left condyle final: " << leftCondyleFinal[0] << "," << leftCondyleFinal[1] << "," << leftCondyleFinal[2] << std::endl;
	std::cout << "[Condyles] Right condyle final: " << rightCondyleFinal[0] << "," << rightCondyleFinal[1] << "," << rightCondyleFinal[2] << std::endl;
}


void MandibularBaseAlgorithm::GetLeft2NasionDifferenceEx(double** arr, int* size)
{
	this->GetPoint(left2NasionDiff, arr, size);
}
void MandibularBaseAlgorithm::GetRight2NasionDifferenceEx(double** arr, int* size)
{
	this->GetPoint(right2NasionDiff, arr, size);
}

int MandibularBaseAlgorithm::GetMeanZCoord(ImageType* binaryInput)
{
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	using IteratorType = itk::ImageRegionIterator<ImageType>;

	auto inputRegion = binaryInput->GetLargestPossibleRegion();
	auto iterator = ConstIteratorType(binaryInput, inputRegion);
	iterator.GoToBegin();

	int count = 0;
	double sum = 0;
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _selectionValue)
		{
			sum += iterator.GetIndex()[2];
			count++;
		}
		++iterator;
	}
	sum /= count;
	return sum;
}

int MandibularBaseAlgorithm::GetMeanZCoord(std::vector<ImageType::IndexType> points)
{
	int count = 0;
	double sum = 0;
	for (auto it = points.begin(); it != points.end(); it++)
	{
		sum += (*it)[2];
		count++;
	}
	sum /= count;
	return sum;
}

void MandibularBaseAlgorithm::RemovePointsAbove(ImageType* image, int maxZ)
{
	ImageType::RegionType inputRegion;
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	using IteratorType = itk::ImageRegionIterator<ImageType>;

	ImageType::RegionType::IndexType inputStart;
	ImageType::RegionType::SizeType size;

	auto largestSize = image->GetLargestPossibleRegion().GetSize();
	inputStart = { 0, 0, maxZ };
	size = { largestSize[0], largestSize[1], largestSize[2] - maxZ };
	inputRegion.SetSize(size);
	inputRegion.SetIndex(inputStart);

	auto iterator = IteratorType(image, inputRegion);
	iterator.GoToBegin();

	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _selectionValue)
			iterator.Set(SHRT_MIN);
		++iterator;
	}
}

std::vector<MandibularBaseAlgorithm::ImageType::IndexType>
MandibularBaseAlgorithm::GetPointsBelowZ(ImageType* localInput, std::vector<std::vector<int>> boundingBoxXYZ, int maxZ)
{
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	using IteratorType = itk::ImageRegionIterator<ImageType>;

	auto inputRegion = localInput->GetLargestPossibleRegion();
	auto iterator = ConstIteratorType(localInput, inputRegion);
	iterator.GoToBegin();

	auto points = std::vector<ImageType::IndexType>();
	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _selectionValue)
		{
			auto index = iterator.GetIndex();
			if (index[0] > boundingBoxXYZ[0][0]
				&& index[1] > boundingBoxXYZ[0][1]
				&& index[0] < boundingBoxXYZ[1][0]
				&& index[1] < boundingBoxXYZ[1][1]
				&& index[2] < maxZ)
			{
				points.push_back(index);
			}
		}
		++iterator;
	}
	return points;
}

long MandibularBaseAlgorithm::GetVoxelCount()
{
	return this->voxelCount;
}

float MandibularBaseAlgorithm::GetProgress()
{
	return progressAccumulator->GetAccumulatedProgress();
}

void MandibularBaseAlgorithm::GetLeftCondylePositionEX(double** arr, int* size)
{
	return this->GetPoint(leftCondylePos, arr, size);
}
void MandibularBaseAlgorithm::GetRightCondylePositionEX(double** arr, int* size)
{
	return this->GetPoint(rightCondylePos, arr, size);
}
void MandibularBaseAlgorithm::GetCondylesDifferenceEX(double** arr, int* size)
{
	return this->GetPoint(condylesPosDiff, arr, size);
}
void MandibularBaseAlgorithm::GetNasionPointEX(double** arr, int* size)
{
	return this->GetPoint(nasionPoint, arr, size);
}
void MandibularBaseAlgorithm::GetLeftZygomaticPointEx(double** arr, int* size)
{
	return this->GetPoint(leftZygomaticPoint, arr, size);
}
void MandibularBaseAlgorithm::GetRightZygomaticPointEx(double** arr, int* size)
{
	return this->GetPoint(rightZygomaticPoint, arr, size);
}

void MandibularBaseAlgorithm::GetPoint(std::vector<int> pt, double** targetArr, int* size)
{
	auto length = pt.size();
	*size = length;
	*targetArr = new double[length];
	for (int i = 0; i < length; i++)
		(*targetArr)[i] = pt[i];
}
void MandibularBaseAlgorithm::GetPoint(std::vector<double> pt, double** targetArr, int* size)
{
	auto length = pt.size();
	*size = length;
	*targetArr = new double[length];
	for (int i = 0; i < length; i++)
		(*targetArr)[i] = pt[i];
}

MandibularBaseAlgorithm::~MandibularBaseAlgorithm()
{
	std::cout << "MandibularBaseAlgorithm dctor..." << std::endl;
	if (leftLinesImage != nullptr)
	{
		delete leftLinesImage;
		leftLinesImage = nullptr;
	}
	if (rightLinesImage != nullptr)
	{
		delete rightLinesImage;
		rightLinesImage = nullptr;
	}
	if (upperSkullProjection != nullptr)
	{
		delete upperSkullProjection;
		upperSkullProjection = nullptr;
	}
	if (topCutProjection != nullptr)
	{
		delete topCutProjection;
		topCutProjection = nullptr;
	}
	if (topCutFilteredProjection != nullptr)
	{
		delete topCutFilteredProjection;
		topCutFilteredProjection = nullptr;
	}
	if (archSpacesProjection != nullptr)
	{
		delete archSpacesProjection;
		archSpacesProjection = nullptr;
	}
	if (archsProjection != nullptr)
	{
		delete archsProjection;
		archsProjection = nullptr;
	}
}