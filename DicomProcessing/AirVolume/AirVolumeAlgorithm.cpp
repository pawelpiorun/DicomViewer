#include "AirVolumeAlgorithm.h"
#include "SkeletonToGraphFilter.h"
#include "MinimalCrossSectionFinder.h"
#include "helpers.h"

AirVolumeAlgorithm::AirVolumeAlgorithm()
{
	progressAccumulator = itk::ProgressAccumulator::New();

	medianFilter = MedianFilterType::New();
	medianFilter->SetRadius(1);
	medianFilter->SetReleaseDataFlag(true);

	threshold = ThresholdType::New();
	threshold->SetInput(medianFilter->GetOutput());
	threshold->SetUpperThreshold(-300);
	threshold->SetInsideValue(_selectionValue);

	erode = ErodeType::New();
	ErodeKernelType::RadiusType erodeRadius;
	erodeRadius[0] = 12; erodeRadius[1] = 12; erodeRadius[2] = 1;
	auto erodeKernel = ErodeKernelType::Box(erodeRadius);
	erode->SetForegroundValue(_selectionValue);
	erode->SetKernel(erodeKernel);
	erode->SetInput(threshold->GetOutput());
	erode->SetReleaseDataFlag(true);

	connectedThreshold = ConnectedThresholdType::New();
	connectedThreshold->SetInput(erode->GetOutput());
	connectedThreshold->SetLower(0);
	connectedThreshold->SetReplaceValue(_selectionValue);
	erode->SetReleaseDataFlag(true);

	dilate = DilateType::New();
	DilateKernelType::RadiusType dilateRadius;
	dilateRadius[0] = 15; dilateRadius[1] = 15; dilateRadius[2] = 1;
	auto dilateKernel = ErodeKernelType::Box(dilateRadius);
	dilate->SetForegroundValue(_selectionValue);
	dilate->SetKernel(dilateKernel);
	dilate->SetInput(connectedThreshold->GetOutput());
	dilate->SetReleaseDataFlag(true);

	subtract = SubtractFilterType::New();
	subtract->SetInput1(threshold->GetOutput());
	subtract->SetInput2(dilate->GetOutput());
	subtract->SetReleaseDataFlag(true);

	keepNFilter = KeepNFilterType::New();
	keepNFilter->SetInput(subtract->GetOutput());
	keepNFilter->SetAttribute("PhysicalSize");
	keepNFilter->SetForegroundValue(_selectionValue);
	keepNFilter->SetNumberOfObjects(1);

	//closing
	closingFilter = ClosingFilter::New();
	ClosingKernelType::RadiusType closingRadius;
	closingRadius[0] = 1; closingRadius[1] = 1; closingRadius[2] = 1;
	auto closingKernel = ClosingKernelType::Ball(closingRadius);
	closingFilter->SetRadius(closingRadius);
	closingFilter->SetInput(keepNFilter->GetOutput());
	closingFilter->SetReleaseDataFlag(true);

	keepAfterCutFilter = KeepNFilterType::New();
	keepAfterCutFilter->SetInput(closingFilter->GetOutput());
	keepAfterCutFilter->SetAttribute("PhysicalSize");
	keepAfterCutFilter->SetForegroundValue(_selectionValue);
	keepAfterCutFilter->SetNumberOfObjects(1);

	skeletonizeFilter = SkeletonizeFilter::New();
	skeletonizeFilter->SetInput(keepAfterCutFilter->GetOutput());

	skeletonSimplify = SkeletonSimplifyFilter::New();
	skeletonSimplify->SetInput(skeletonizeFilter->GetOutput());

	volumePartitioning = VolumePartitioningFilter::New();
	volumePartitioning->SetInput(keepAfterCutFilter->GetOutput());

	keepAfterPartitioning = KeepNFilterType::New();
	keepAfterPartitioning->SetInput(volumePartitioning->GetOutput());
	keepAfterPartitioning->SetAttribute("PhysicalSize");
	keepAfterPartitioning->SetForegroundValue(_selectionValue);
	keepAfterPartitioning->SetNumberOfObjects(1);

	labelStatistics = LabelStatisticsFilter::New();
	labelStatistics->SetInput(keepAfterPartitioning->GetOutput());
	labelStatistics->SetLabelInput(keepAfterPartitioning->GetOutput());
	labelStatistics->SetReleaseDataFlag(true);

	progressAccumulator->RegisterInternalFilter(threshold, 0.05f);
	progressAccumulator->RegisterInternalFilter(erode, 0.1f);
	progressAccumulator->RegisterInternalFilter(connectedThreshold, 0.1f);
	progressAccumulator->RegisterInternalFilter(dilate, 0.1f);
	progressAccumulator->RegisterInternalFilter(subtract, 0.05f);
	progressAccumulator->RegisterInternalFilter(keepNFilter, 0.05f);
	progressAccumulator->RegisterInternalFilter(keepAfterCutFilter, 0.05f);
	progressAccumulator->RegisterInternalFilter(skeletonizeFilter, 0.2f);
	progressAccumulator->RegisterInternalFilter(skeletonSimplify, 0.05f);
	progressAccumulator->RegisterInternalFilter(volumePartitioning, 0.2f);
	progressAccumulator->RegisterInternalFilter(keepAfterPartitioning, 0.05f);
	//progressAccumulator->RegisterInternalFilter(labelStatistics, 0.01f);

	SetLastPipelineObject(keepAfterPartitioning);
	lastPipeline->SetReleaseDataFlag(false);
}

void AirVolumeAlgorithm::GenerateData()
{
	auto tempFolder = std::string(helpers::GetAppDataTempPath());

	auto localInput = DicomSeriesType::New();
	localInput->Graft(const_cast<DicomSeriesType*>(this->GetInput()));
	progressAccumulator->SetMiniPipelineFilter(this);

	medianFilter->SetInput(localInput);

	if (lastPipelineIndex == MedianIndex)
	{
		try
		{
			medianFilter->Update();
		}
		catch (itk::ExceptionObject x)
		{
			std::cout << x.GetDescription() << std::endl;
		}
		this->GraftOutput(medianFilter->GetOutput());
		return;
	}

	auto corners = DicomProcessing::GetCorners(localInput);
	for (int i = 0; i < corners.size(); i++)
		connectedThreshold->AddSeed(corners[i]);

	if (lastPipelineIndex >= SubtractIndex)
	{
		try
		{
			dilate->Update();
			threshold->SetReleaseDataFlag(true);
		}
		catch (itk::ExceptionObject x)
		{
			std::cout << x.GetDescription() << std::endl;
		}
	}

	if (lastPipelineIndex < KeepNFilterIndex)
	{
		try
		{
			lastPipeline->Update();
		}
		catch (itk::ExceptionObject x)
		{
			std::cout << x.GetDescription() << std::endl;
		}
		this->GraftOutput(lastPipeline->GetOutput());
		return;
	}

	try
	{
		keepNFilter->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}

	frontProjection = GetAirVolumeProjection(keepNFilter->GetOutput(), FrontProjection);
	frontProjection->Swap();

	sideProjection = GetAirVolumeProjection(keepNFilter->GetOutput());
	sideProjection->Swap();

	// TODO - abort saving
	frontProjection->SaveCompressed((tempFolder + "\\AirVolume_FrontProjection.jpg").c_str());
	sideProjection->SaveCompressed((tempFolder + "\\AirVolume_SideProjection.jpg").c_str());

	if (lastPipelineIndex == KeepNFilterIndex)
	{
		this->GraftOutput(keepNFilter->GetOutput());
		return;
	}

	// sinuses
	auto mediumPoint = DicomProcessing::GetMediumPoint(sideProjection->GetItkDicomImage(), _selectionValue);
	auto sinusCutLinePoints = GetSinusCutLinePoints(sideProjection, mediumPoint[1]);
	auto sinusCutLine = CreateLine(sinusCutLinePoints[0], sinusCutLinePoints[1]);
	//SaveLineImage(tempSideProjection, sinusCutLine, ("C:\\temp\\proj - line1 - " + tempSideProjection->GetImageId() + ".dcm").c_str());

	// epiglottis
	auto lowerLeftHalfSideProjection = GetAirVolumeLowerHalfProjection(keepNFilter->GetOutput());
	auto lowerRightHalfSideProjection = GetAirVolumeLowerHalfProjection(keepNFilter->GetOutput(), true);
	lowerLeftHalfSideProjection->Swap();
	lowerRightHalfSideProjection->Swap();
	auto leftEpiglottisCutLinePoints = GetEpiglottisCutLinePoints(lowerLeftHalfSideProjection);
	auto leftEpiglottisCutLine = CreateLine(leftEpiglottisCutLinePoints[0], leftEpiglottisCutLinePoints[1]);
	auto rightEpiglottisCutLinePoints = GetEpiglottisCutLinePoints(lowerRightHalfSideProjection);
	auto rightEpiglottisCutLine = CreateLine(rightEpiglottisCutLinePoints[0], rightEpiglottisCutLinePoints[1]);

	std::cout << "[EpiglottisLine] Epiglottis from right: " << rightEpiglottisCutLine[1] << std::endl;
	std::cout << "[EpiglottisLine] Epiglottis from left: " << leftEpiglottisCutLine[1] << std::endl;

	std::vector<double> epiglottisCutLine;
	std::vector<DicomImageType::IndexType> epiglottisCutLinePoints;
	if (rightEpiglottisCutLine[1] < leftEpiglottisCutLine[1])
	{
		epiglottisCutLine = rightEpiglottisCutLine;
		epiglottisCutLinePoints = rightEpiglottisCutLinePoints;
	}
	else
	{
		epiglottisCutLine = leftEpiglottisCutLine;
		epiglottisCutLinePoints = leftEpiglottisCutLinePoints;
	}

	// swap back
	auto swappedEpiglottisPoints = SwapPoints(sideProjection, epiglottisCutLinePoints);
	auto swappedLineEpiglottis = CreateLine(swappedEpiglottisPoints[0], swappedEpiglottisPoints[1]);
	std::cout << "[EpiglottisLine] Epiglottis line: y = " << swappedLineEpiglottis[0] << "x + " << swappedLineEpiglottis[1] << std::endl;

	auto swappedSinusPoints = SwapPoints(sideProjection, sinusCutLinePoints);
	auto swappedLineSinus = CreateLine(swappedSinusPoints[0], swappedSinusPoints[1]);
	std::cout << "[SinusesLine] Sinus cut line: y = " << swappedLineSinus[0] << "x + " << swappedLineSinus[1] << std::endl;

	RemoveVoxelsBelowEpiglottis(keepNFilter->GetOutput(), swappedLineEpiglottis);
	RemoveSinusesVoxels(keepNFilter->GetOutput(), swappedLineSinus, swappedSinusPoints);
	//OpenSinusesVoxels(keepNFilter->GetOutput(), swappedLineSinus, swappedSinusPoints);

	
	auto tempSideProjection = GetAirVolumeProjection(keepNFilter->GetOutput(), SideProjection);
	tempSideProjection->Swap();
	auto oroPharynxPoints = GetOroPharynxCutLinePoints(tempSideProjection);
	auto swappedOroPharynxPoints = SwapPoints(tempSideProjection, oroPharynxPoints);
	RemoveOroPharynxVoxels(keepNFilter->GetOutput(), swappedOroPharynxPoints);
	if (swappedOroPharynxPoints.empty())
		std::cout << "[OroPharynx] Cut aborted." << std::endl;
	else
		std::cout << "[OroPharynx] Cut point: (y,z) = (" << swappedOroPharynxPoints[0][0] << "," << swappedOroPharynxPoints[0][1] << ")" << std::endl;
	delete tempSideProjection;
	tempSideProjection = nullptr;

	auto oroPharynxCutProjection = GetAirVolumeProjection(keepNFilter->GetOutput());
	oroPharynxCutProjection->Swap();
	oroPharynxCutProjection->SaveCompressed((tempFolder + "\\AirVolume_OroPharynxCutProjection.jpg").c_str());
	delete oroPharynxCutProjection;
	oroPharynxCutProjection = nullptr;

	try
	{
		keepNFilter->SetReleaseDataFlag(true);
		keepAfterCutFilter->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}

	if (lastPipelineIndex == KeepAfterCutIndex)
	{
		this->GraftOutput(keepAfterCutFilter->GetOutput());
		return;
	}

	{
		using IteratorType = itk::ImageRegionIterator<ImageType>;
		auto keepAfter = keepAfterCutFilter->GetOutput();
		auto keepAfterIt = IteratorType(keepAfter, keepAfter->GetLargestPossibleRegion());
		keepAfterIt.GoToBegin();

		while (!keepAfterIt.IsAtEnd())
		{
			auto val = keepAfterIt.Value();
			if (val == _selectionValue)
				keepAfterIt.Set(1);
			else
				keepAfterIt.Set(0);
			++keepAfterIt;
		}
	}

	try
	{
		skeletonizeFilter->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}

	{
		using IteratorType = itk::ImageRegionIterator<ImageType>;
		auto cutInput = keepAfterCutFilter->GetOutput();
		auto inputIt = IteratorType(cutInput, cutInput->GetLargestPossibleRegion());
		inputIt.GoToBegin();

		while (!inputIt.IsAtEnd())
		{
			auto val = inputIt.Value();
			if (val == 1)
				inputIt.Set(_selectionValue);
			++inputIt;
		}

		auto skeletonized = skeletonizeFilter->GetOutput();
		auto it = IteratorType(skeletonized, skeletonized->GetLargestPossibleRegion());
		it.GoToBegin();

		while (!it.IsAtEnd())
		{
			auto val = it.Value();
			if (val == 1)
				it.Set(_selectionValue);
			++it;
		}

		skeletonFrontProjection = GetAirVolumeProjection(skeletonized, FrontProjection);
		skeletonFrontProjection->Swap();
		skeletonFrontProjection->SetFileName("AirVolume_SkeletonProjection_Front");

		skeletonSideProjection = GetAirVolumeProjection(skeletonized);
		skeletonSideProjection->Swap();
		skeletonSideProjection->SetFileName("AirVolume_SkeletonProjection_Side");

		// todo - abort saving
		skeletonFrontProjection->SaveCompressed((tempFolder + "\\AirVolume_SkeletonProjection_Front.jpg").c_str());
		skeletonSideProjection->SaveCompressed((tempFolder + "\\AirVolume_SkeletonProjection_Side.jpg").c_str());
	}

	if (lastPipelineIndex == SkeletonizeIndex)
	{
		this->GraftOutput(skeletonizeFilter->GetOutput());
		return;
	}

	auto skeletonToGraph = new SkeletonToGraph();
	skeletonToGraph->SetInput(skeletonizeFilter->GetOutput());
	auto graph = skeletonToGraph->GetGraph();
	skeletonSimplify->SetSkeletonGraph(graph);
	skeletonSimplify->SetSinusesLine(swappedLineSinus, swappedSinusPoints);

	try
	{
		skeletonizeFilter->SetReleaseDataFlag(true);
		skeletonSimplify->Update();
		delete skeletonToGraph;
		skeletonToGraph = nullptr;
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}

	simplifiedSkeletonFrontProjection = GetAirVolumeProjection(skeletonSimplify->GetOutput(), FrontProjection);
	simplifiedSkeletonFrontProjection->Swap();
	simplifiedSkeletonFrontProjection->SetFileName("AirVolume_SimplifiedSkeletonProjection_Front");
	
	simplifiedSkeletonSideProjection = GetAirVolumeProjection(skeletonSimplify->GetOutput());
	simplifiedSkeletonSideProjection->Swap();
	simplifiedSkeletonSideProjection->SetFileName("AirVolume_SimplifiedSkeletonProjection_Side");

	auto minimalPathsImage = skeletonSimplify->GetMinimalPathsImage();
	auto mpv = skeletonSimplify->GetMinimalPathsVoxels();
	std::copy(mpv.begin(), mpv.end(), std::back_inserter(minimalPathsVoxels));

	minimalPathsFront = GetAirVolumeProjection(minimalPathsImage->GetItkImageData(), FrontProjection);
	minimalPathsFront->Swap();
	minimalPathsFront->SetFileName("AirVolume_MinimalPaths_Front");
	minimalPathsSide = GetAirVolumeProjection(minimalPathsImage->GetItkImageData());
	minimalPathsSide->Swap();
	minimalPathsSide->SetFileName("AirVolume_MinimalPaths_Side");

	// TODO - abort saving
	simplifiedSkeletonSideProjection->SaveCompressed((tempFolder + "\\AirVolume_SimplifiedSkeletonProjection_Side.jpg").c_str());
	simplifiedSkeletonFrontProjection->SaveCompressed((tempFolder + "\\AirVolume_SimplifiedSkeletonProjection_Front.jpg").c_str());
	minimalPathsFront->SaveCompressed((tempFolder + "\\AirVolume_MinimalPaths_Front.jpg").c_str());
	minimalPathsSide->SaveCompressed((tempFolder + "\\AirVolume_MinimalPaths_Side.jpg").c_str());

	if (lastPipelineIndex == SimplifySkeletonIndex)
	{
		this->GraftOutput(skeletonSimplify->GetOutput());
		return;
	}

	//auto simplifiedGraph = skeletonSimplify->GetSimplifiedGraph();
	auto removedVertices = skeletonSimplify->GetRemovedVertices();
	volumePartitioning->SetSkeletonGraph(graph);
	volumePartitioning->SetRemovedVertices(removedVertices);
	volumePartitioning->SetUseRemovedVertices(true);
	volumePartitioning->SetUseMinimalPaths(false);
	//volumePartitioning->SetMinimalPaths(minimalPaths);

	try
	{
		keepAfterCutFilter->SetReleaseDataFlag(true);
		keepAfterPartitioning->Update();
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}

	auto minimalPaths = skeletonSimplify->GetMinimalPaths();

	auto minimalSectionFinder = new MinimalCrossSectionFinder();
	minimalSectionFinder->SetInput(keepAfterPartitioning->GetOutput());
	minimalSectionFinder->SetMinimalPathsImage(minimalPathsImage->GetItkImageData());
	minimalSectionFinder->SetMinimalPaths(minimalPaths);
	minimalSectionFinder->Compute();

	minimalCrossSection = minimalSectionFinder->GetMinimalCrossSection();
	minimalCrossSection2 = minimalSectionFinder->GetMinimalCrossSection2();
	minimalCrossSection3 = minimalSectionFinder->GetMinimalCrossSection3();
	minimalCrossSection4 = minimalSectionFinder->GetMinimalCrossSection4();
	minimalCrossSections = minimalSectionFinder->GetMinimalCrossSections();
	minimalCrossSections2 = minimalSectionFinder->GetMinimalCrossSections2();
	minimalCrossSections3 = minimalSectionFinder->GetMinimalCrossSections3();
	minimalCrossSections4 = minimalSectionFinder->GetMinimalCrossSections4();
	minimalCrossSectionsCoords = minimalSectionFinder->GetMinimalCrossSectionsCoords();
	minimalCrossSectionsNormals = minimalSectionFinder->GetMinimalCrossSectionsNormals();
	frontCrossSection = minimalSectionFinder->GetMinimalCrossSectionFrontImage();
	frontCrossSection->Swap();
	sideCrossSection = minimalSectionFinder->GetMinimalCrossSectionSideImage();
	sideCrossSection->Swap();

	// TODO - abort saving
	sideCrossSection->Save((tempFolder + "\\AirVolume_CrossSection_Side.jpg").c_str());
	frontCrossSection->Save((tempFolder + "\\AirVolume_CrossSection_Front.jpg").c_str());

	auto splitPoint = minimalSectionFinder->GetSplitPlanePoint();
	auto splitPlane = minimalSectionFinder->GetSplitPlaneNormal();
	delete minimalSectionFinder;
	minimalSectionFinder = nullptr;

	try
	{
		//skeletonSimplify->SetReleaseDataFlag(true);
		skeletonSimplify->ReleaseData();
		keepAfterPartitioning->SetReleaseDataFlag(true);
		labelStatistics->Update();

		std::cout << "Clearing source graph... " << std::endl;
		if (graph != nullptr)
		{
			delete graph;
			graph = nullptr;
		}
		std::cout << "Done." << std::endl;
	}
	catch (itk::ExceptionObject x)
	{
		std::cout << x.GetDescription() << std::endl;
	}

	this->pharynxVoxelCount = GetPharynxVoxelCount(keepAfterPartitioning->GetOutput(), splitPoint, splitPlane);

	auto validLabels = labelStatistics->GetValidLabelValues();
	this->voxelCount = labelStatistics->GetCount(_selectionValue);
	this->nasalCavityVoxelCount = voxelCount - pharynxVoxelCount;
	
	std::cout << "[LabelStatistics] Voxel count: " << voxelCount << std::endl;
	std::cout << "[LabelStatistics] Pharynx voxel count: " << pharynxVoxelCount << std::endl;
	std::cout << "[LabelStatistics] Nasal cavity count: " << nasalCavityVoxelCount << std::endl;

	auto spacing = localInput->GetSpacing();
	std::cout << "[LabelStatistics] Spacing: " << spacing[0] << " x "
		<< spacing[1] << " x " << spacing[2] << " mm" << std::endl;

	auto pixelVolume = spacing[0] * spacing[1] * spacing[2];
	calculatedVolume = voxelCount * pixelVolume / 1000;
	calculatedPharynx = pharynxVoxelCount * pixelVolume / 1000;
	calculatedNasalCavity = nasalCavityVoxelCount * pixelVolume / 1000;
	std::cout << "[LabelStatistics] Calculated volume: " << calculatedVolume << " cm3 (NC: " << calculatedNasalCavity << " PH: " << calculatedPharynx << std::endl;
	std::cout << "[LabelStatistics] Number of labels: " << labelStatistics->GetNumberOfLabels() << std::endl;
	auto labels = labelStatistics->GetValidLabelValues();
	std::cout << "[LabelStatistics] Labels: ";
	for (auto lit = labels.begin(); lit != labels.end(); lit++)
	{
		std::cout << *lit << "\t";
	}
	std::cout << std::endl;

	this->GraftOutput(lastPipeline->GetOutput());
}

void AirVolumeAlgorithm::Run(DicomSeries* dicomSeries, bool keepOriginalImage)
{
	this->SetInput(dicomSeries->GetItkImageData());
	input = dicomSeries;
	if (!keepOriginalImage)
	{
		dicomSeries->SetItkImageData(this->GetOutput());
	}
	this->Update();

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
				dstIt.Set(3000);
				count++;
			}
			else if (dstIt.Get() > 2000)
				dstIt.Set(2000);
			++srcIt;
			++dstIt;
		}

		for (int i = 0; i < minimalPathsVoxels.size(); i++)
			imageData->SetPixel(minimalPathsVoxels[i], _selectionValue);

		std::cout << std::endl << "[Air volume] Selected points: " << count << std::endl;
	}
}

DicomImage* AirVolumeAlgorithm::GetAirVolumeProjection(ImageType::Pointer airVolume, int projectionType)
{
	// volume iterator
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto volumeRegion = airVolume->GetLargestPossibleRegion();
	auto volumeSize = volumeRegion.GetSize();
	auto volumeSpacing = airVolume->GetSpacing();
	auto constIt = ConstIteratorType(airVolume, volumeRegion);
	constIt.GoToBegin();

	// new projection image
	auto dicomImage = DicomImageType::New();
	auto region = DicomImageType::RegionType();
	DicomImageType::IndexType projectionStart = { 0, 0 };
	DicomImageType::SizeType projectionSize;
	DicomImageType::SpacingType imageSpacing = itk::Vector<double, 2U>();
	if (projectionType == SideProjection)
	{
		projectionSize = { volumeSize[1], volumeSize[2] };
		imageSpacing[0] = volumeSpacing[1];
		imageSpacing[1] = volumeSpacing[2];

	}
	else if (projectionType == FrontProjection)
	{
		projectionSize = { volumeSize[0], volumeSize[2] };
		imageSpacing[0] = volumeSpacing[0];
		imageSpacing[1] = volumeSpacing[2];
	}
	else
	{
		projectionSize = { volumeSize[0], volumeSize[1] };
		imageSpacing[0] = volumeSpacing[0];
		imageSpacing[1] = volumeSpacing[1];
	}
	region.SetSize(projectionSize);
	region.SetIndex(projectionStart);
	dicomImage->SetRegions(region);
	dicomImage->Allocate();
	dicomImage->FillBuffer(0);
	dicomImage->SetSpacing(imageSpacing);

	while (!constIt.IsAtEnd())
	{
		if (constIt.Value() == _selectionValue)
		{
			// set projection image pixel
			auto index = constIt.GetIndex();

			DicomImageType::IndexType projectedIndex;
			if (projectionType == SideProjection)
				projectedIndex = { index[1], index[2] };
			else if (projectionType == FrontProjection)
				projectedIndex = { index[0], index[2] };
			else
				projectedIndex = { index[0], index[1] };
			dicomImage->SetPixel(projectedIndex, _selectionValue);
		}
		++constIt;
	}

	auto image = new DicomImage();
	image->SetItkDicomImage(dicomImage);
	image->CopyInformation(input);
	auto name = "AirVolume_SideProjection";
	if (projectionType == FrontProjection)
		name = "AirVolume_FrontProjection";
	else if (projectionType == TopProjection)
		name = "AirVolume_TopProjection";
	image->SetFileName(name);
	return image;
}

DicomImage* AirVolumeAlgorithm::GetAirVolumeLowerHalfProjection(ImageType::Pointer airVolume, bool right)
{
	auto mediumPoint = DicomProcessing::GetMediumPoint(airVolume, _selectionValue);
	auto zCut = mediumPoint[2];
	mediumPoint = DicomProcessing::GetMediumPointBelowZ(airVolume, _selectionValue, zCut);

	// volume iterator
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto volumeRegion = airVolume->GetLargestPossibleRegion();
	auto volumeSize = volumeRegion.GetSize();
	auto volumeSpacing = airVolume->GetSpacing();
	auto constIt = ConstIteratorType(airVolume, volumeRegion);
	constIt.GoToBegin();

	// new projection image, but only half of lower air
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
		if (constIt.Value() == _selectionValue
			&& ((right && constIt.GetIndex()[0] < mediumPoint[0]) || (!right && constIt.GetIndex()[0] > mediumPoint[0]))
			&& constIt.GetIndex()[2] < zCut)
		{
			// set projection image pixel
			auto index = constIt.GetIndex();
			DicomImageType::IndexType projectedIndex = { index[1], index[2] };
			dicomImage->SetPixel(projectedIndex, _selectionValue);
		}
		++constIt;
	}

	auto image = new DicomImage();
	image->SetItkDicomImage(dicomImage);
	image->CopyInformation(input);
	image->SetFileName("AirVolume_LowerHalfSideProjection");
	return image;
}

std::vector<DicomImageType::IndexType> AirVolumeAlgorithm::GetEpiglottisCutLinePoints(DicomImage* projection)
{
	auto dicomImage = projection->GetItkDicomImage();

	using ConstLinearIteratorType = itk::ImageLinearConstIteratorWithIndex<DicomImageType>;
	auto region = dicomImage->GetLargestPossibleRegion();
	auto iterator = ConstLinearIteratorType(dicomImage, region);
	iterator.GoToReverseBegin();

	DicomImageType::IndexType previousLineStart = iterator.GetIndex();
	iterator.PreviousLine();

	bool afterLineWithGap = false;
	int lineWithGapCount = 0;
	while (!iterator.IsAtReverseEnd())
	{
		iterator.GoToBeginOfLine();
		bool isGap = true;
		int lineCount = 0;

		int singleLineVoxelCount = 0;
		while (!iterator.IsAtEndOfLine())
		{
			if (iterator.Value() == _selectionValue)
			{
				singleLineVoxelCount++;
				if (isGap && singleLineVoxelCount > 3)
				{
					isGap = false;
					lineCount++;
				}
			}
			else
			{
				isGap = true;
				singleLineVoxelCount = 0;
			}

			++iterator;
		}

		if (!afterLineWithGap && lineCount > 1)
		{
			lineWithGapCount++;
			if (lineWithGapCount == 3)
				afterLineWithGap = true;
		}

		if (lineCount == 1 && afterLineWithGap)
			break;

		iterator.PreviousLine();
	}

	auto coord = iterator.GetIndex()[1];
	auto points = std::vector<DicomImageType::IndexType>();
	DicomImageType::IndexType first = { 0, coord };
	DicomImageType::IndexType second = { region.GetSize()[0] - 1, coord };
	points.push_back(first);
	points.push_back(second);

	return points;
}


std::vector<DicomImageType::IndexType> AirVolumeAlgorithm::GetOroPharynxCutLinePoints(DicomImage* projection)
{
	auto dicomImage = projection->GetItkDicomImage();

	using ConstLinearIteratorType = itk::ImageLinearConstIteratorWithIndex<DicomImageType>;
	auto region = dicomImage->GetLargestPossibleRegion();
	auto iterator = ConstLinearIteratorType(dicomImage, region);
	iterator.GoToReverseBegin();

	DicomImageType::IndexType previousLineStart = iterator.GetIndex();
	iterator.PreviousLine();

	// find first line with gap
	bool startCheck = false;
	auto gapY = -1;
	while (!iterator.IsAtReverseEnd())
	{
		iterator.GoToBeginOfLine();
		bool isGap = true;
		int lineCount = 0;

		int singleLineVoxelCount = 0;
		int singleGapVoxelCount = 0;
		while (!iterator.IsAtEndOfLine())
		{
			auto val = iterator.Value();
			if (val == _selectionValue && !startCheck)
			{
				startCheck = true;
			}
			else if (startCheck && val == _selectionValue)
			{
				if (lineCount > 0 && singleGapVoxelCount > 10)
				{
					std::cout << "[OroPharynx] Huge gap. Aborting." << std::endl;
					return std::vector<DicomImageType::IndexType>();
				}

				singleGapVoxelCount = 0;
				singleLineVoxelCount++;
				if (isGap && singleLineVoxelCount > 3)
				{
					isGap = false;
					lineCount++;
				}
			}
			else if (startCheck)
			{
				if (singleGapVoxelCount > 2 && gapY == -1)
					gapY = iterator.GetIndex()[0] - 3;
				isGap = true;
				singleLineVoxelCount = 0;
				singleGapVoxelCount++;
			}

			++iterator;
		}

		if (lineCount > 1)
			break;

		iterator.PreviousLine();
	}

	auto coordZ = iterator.GetIndex()[1];
	auto points = std::vector<DicomImageType::IndexType>();
	DicomImageType::IndexType first = { gapY, coordZ };
	points.push_back(first);

	return points;
}

std::vector<DicomImageType::IndexType>
AirVolumeAlgorithm::GetSinusCutLinePoints(DicomImage* projection, int mediumZ)
{
	auto dicomImage = projection->GetItkDicomImage();
	using ConstIteratorType = itk::ImageLinearConstIteratorWithIndex<DicomImageType>;
	auto region = dicomImage->GetLargestPossibleRegion();
	auto iterator = ConstIteratorType(dicomImage, region);
	iterator.GoToReverseBegin();

	while (iterator.GetIndex()[1] > mediumZ)
		iterator.PreviousLine();
	iterator.GoToBeginOfLine();
	DicomImageType::IndexType previousLineStart = iterator.GetIndex();

	auto tolerance = region.GetSize()[0] * 0.0075;
	while (!iterator.IsAtReverseEnd())
	{
		iterator.GoToBeginOfLine();
		DicomImageType::IndexType lineStart = DicomImageType::IndexType();
		while (!iterator.IsAtEndOfLine())
		{
			if (iterator.Value() == _selectionValue)
			{
				lineStart = iterator.GetIndex();
				break;
			}

			++iterator;
		}

		if (!iterator.IsAtEndOfLine())
		{
			auto prevX = previousLineStart[0];
			auto thisX = lineStart[0];

			if (prevX - thisX > tolerance)
				break;
		}
		previousLineStart = lineStart;

		iterator.PreviousLine();
	}
	auto firstPoint = previousLineStart;

	bool isFirstPointFound = false;
	iterator.GoToBegin();
	iterator.GoToReverseBeginOfLine();
	previousLineStart = iterator.GetIndex();
	while (!iterator.IsAtEnd())
	{
		iterator.GoToReverseBeginOfLine();
		DicomImageType::IndexType lineStart = DicomImageType::IndexType();
		while (!iterator.IsAtReverseEndOfLine())
		{
			if (iterator.Value() == _selectionValue)
			{
				if (!isFirstPointFound)
				{
					previousLineStart = iterator.GetIndex();
					isFirstPointFound = true;
				}
				lineStart = iterator.GetIndex();
				break;
			}

			--iterator;
		}

		if (!iterator.IsAtReverseEndOfLine())
		{
			auto prevX = previousLineStart[0];
			auto thisX = lineStart[0];

			if (std::abs(thisX - prevX) > tolerance)
				break;
		}
		previousLineStart = lineStart;

		iterator.NextLine();
	}
	auto secondPoint = previousLineStart;

	auto points = std::vector<DicomImageType::IndexType>();
	points.push_back(firstPoint);
	points.push_back(secondPoint);

	std::cout << "[SinusesLine] First sinus point: X = " << firstPoint[0] << ", Y = " << firstPoint[1] << std::endl;
	std::cout << "[SinusesLine] Second sinus point: X = " << secondPoint[0] << ", Y = " << secondPoint[1] << std::endl;

	return points;
}

std::vector<DicomImageType::IndexType> AirVolumeAlgorithm::SwapPoints(DicomImage* projection, std::vector<DicomImageType::IndexType> points)
{
	if (points.empty()) return points;

	auto dicomImage = projection->GetItkDicomImage();
	auto region = dicomImage->GetLargestPossibleRegion();
	auto width = region.GetSize()[0];
	auto height = region.GetSize()[1];

	std::vector<DicomImageType::IndexType> swappedPoints = std::vector<DicomImageType::IndexType>();
	for (auto it = points.begin(); it != points.end(); it++)
	{
		auto elem = *it;
		elem[0] = width - elem[0]; elem[1] = height - elem[1];
		swappedPoints.push_back(elem);
	}

	return swappedPoints;
}

void AirVolumeAlgorithm::SaveLineImage(DicomImage* image, std::vector<double> line, const char* outputPath)
{
	auto itkImage = image->GetItkDicomImage();
	auto copy = DicomImageType::New();
	copy->SetRegions(itkImage->GetLargestPossibleRegion());
	copy->Allocate();
	copy->FillBuffer(0);
	copy->SetSpacing(itkImage->GetSpacing());

	auto tolerance = line[0] < 1 ? 1 : line[0] < 5 ? 2 : line[0] < 20 ? 3 : 4;
	using ConstIteratorType = itk::ImageRegionConstIterator<DicomImageType>;
	using IteratorType = itk::ImageRegionIterator<DicomImageType>;
	auto it = IteratorType(copy, copy->GetLargestPossibleRegion());
	auto constIt = ConstIteratorType(itkImage, itkImage->GetLargestPossibleRegion());
	it.GoToBegin();
	constIt.GoToBegin();
	while (!it.IsAtEnd())
	{
		if (constIt.Value() == _selectionValue)
			it.Set(_selectionValue);

		auto in = it.GetIndex();
		auto val = line[0] * in[0] + line[1];
		if (in[1] >= val - tolerance && in[1] <= val + tolerance)
			it.Set(25000);
		++it;
		++constIt;
	}

	auto copyImage = new DicomImage();
	copyImage->SetItkDicomImage(copy);
	auto path = outputPath;
	std::cout << "Saving line image to: " << path << std::endl;
	copyImage->Save(path);
}

void AirVolumeAlgorithm::RemoveSinusesVoxels(
	ImageType::Pointer image, std::vector<double> sinusCutLine, std::vector<DicomImageType::IndexType> sinusCutPoints)
{
	using IteratorType = itk::ImageRegionIterator<ImageType>;
	auto region = image->GetLargestPossibleRegion();
	auto iterator = IteratorType(image, region);
	iterator.GoToBegin();

	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _selectionValue)
		{
			auto in = iterator.GetIndex();
			auto val = in[1] * sinusCutLine[0] + sinusCutLine[1];
			if (in[2] >= sinusCutPoints[0][1] && in[2] >= val)
				iterator.Set(0);
		}

		++iterator;
	}
}

void AirVolumeAlgorithm::OpenSinusesVoxels(
	ImageType::Pointer image, std::vector<double> sinusCutLine, std::vector<DicomImageType::IndexType> sinusCutPoints)
{
	using IteratorType = itk::NeighborhoodIterator<ImageType>;
	auto radius = IteratorType::RadiusType();
	radius.Fill(1);
	auto region = image->GetLargestPossibleRegion();
	auto iterator = IteratorType(radius, image, region);
	iterator.GoToBegin();

	std::vector<ImageType::IndexType> toErode;
	while (!iterator.IsAtEnd())
	{
		if (iterator.GetCenterPixel() == _selectionValue)
		{
			int counter = 0;
			auto in = iterator.GetIndex();
			auto val = in[1] * sinusCutLine[0] + sinusCutLine[1];
			if (in[2] >= sinusCutPoints[0][1] && in[2] <= sinusCutPoints[1][1] && in[2] >= val - 30)
			{
				for (unsigned int i = 0; i < iterator.Size(); i++)
				{
					if (iterator.GetPixel(i) == _selectionValue)
						counter++;
				}
				if (counter < iterator.Size())
					toErode.push_back(in);
			}
		}
		++iterator;
	}

	for (int i = 0; i < toErode.size(); i++)
	{
		image->SetPixel(toErode[i], 0);
	}
}

void AirVolumeAlgorithm::RemoveVoxelsBelowEpiglottis(ImageType::Pointer image, std::vector<double> epiglottisCutLine)
{
	using IteratorType = itk::ImageRegionIterator<ImageType>;
	auto region = image->GetLargestPossibleRegion();
	auto iterator = IteratorType(image, region);
	iterator.GoToBegin();

	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _selectionValue)
		{
			auto in = iterator.GetIndex();
			auto val = in[1] * epiglottisCutLine[0] + epiglottisCutLine[1];
			if (in[2] < val + 3)
				iterator.Set(0);
		}

		++iterator;
	}
}


void AirVolumeAlgorithm::RemoveOroPharynxVoxels(ImageType::Pointer image, std::vector<DicomImageType::IndexType> oroPharynxPoints)
{
	if (oroPharynxPoints.empty()) return;

	auto mainPoint = oroPharynxPoints[0];

	auto region = image->GetLargestPossibleRegion();
	auto size = region.GetSize();
	if (mainPoint[1] > size[2] / 2)
	{
		std::cout << "[OroPharynx] Cut line too hight. Aborting." << std::endl;
		return;
	}

	using IteratorType = itk::ImageRegionIterator<ImageType>;
	auto iterator = IteratorType(image, region);
	iterator.GoToBegin();

	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == _selectionValue)
		{
			auto in = iterator.GetIndex();
			if (std::abs(in[2] - mainPoint[1]) <= 2 && in[1] < mainPoint[0])
				iterator.Set(0);

			if (in[2] < mainPoint[1] - 2 && in[1] < mainPoint[0] - (mainPoint[1] - in[2]))
				iterator.Set(0);
		}

		++iterator;
	}
}

long AirVolumeAlgorithm::GetPharynxVoxelCount(DicomSeriesType::Pointer image, DicomSeriesType::IndexType splitPoint, std::vector<double> splitPlane)
{
	long counter = 0;

	using ConstIteratorType = itk::ImageRegionConstIterator<DicomSeriesType>;
	auto constIt = ConstIteratorType(image, image->GetLargestPossibleRegion());
	constIt.GoToBegin();
	while (!constIt.IsAtEnd())
	{
		if (constIt.Value() == _selectionValue)
		{
			auto in = constIt.GetIndex();
			//auto y = splitPlane[1] * in[2] + splitPoint[1];
			//if (in[1] > y)
			//{
			//	pharynxVoxelCount++;
			//}

			auto x = splitPlane[0] * (in[0] - splitPoint[0]);
			auto y = splitPlane[1] * (in[1] - splitPoint[1]);
			auto z = splitPlane[2] * (in[2] - splitPoint[2]);
			if (x + y + z > 0)
				pharynxVoxelCount++;
		}
		++constIt;
	}

	return pharynxVoxelCount;
}

void AirVolumeAlgorithm::DumpIntermediates(const char* outputFolder)
{
	// ensure output folder
	std::cout << "Dump folder: " + std::string(outputFolder) << std::endl;
	auto patientFolder = std::string(outputFolder) + "\\" + input->GetPatientId() + "_" + input->GetPatientName();
	auto studyFolder = patientFolder + "\\" + input->GetImageId();
	itksys::SystemTools::MakeDirectory(studyFolder);

	auto metadataPath = studyFolder + "\\metadata.txt";
	auto stream = std::ofstream(metadataPath);
	input->Print(stream);

	// front projection
	//frontProjection->Resample();
	auto path = studyFolder + "\\" + frontProjection->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving air volume frontal projection to: " + path << std::endl;
	frontProjection->SaveCompressed(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;

	// side projection
	//sideProjection->Resample();
	path = studyFolder + "\\" + sideProjection->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving air volume side projection to: " << path << std::endl;
	sideProjection->SaveCompressed(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;

	if (lastPipelineIndex < KeepNFilterIndex) return;

	/*lowerHalfSideProjection->Resample();
	auto path2 = studyFolder + "\\" + lowerHalfSideProjection->GetFileName() + ".dcm";
	std::cout << "[AirVolume] Saving air volume lower half side projection to: " << path2 << std::endl;
	lowerHalfSideProjection->Save(path2.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;*/

	if (lastPipelineIndex < SkeletonizeIndex) return;
	
	// skeleton projection
	//skeletonProjection->Resample();
	path = studyFolder + "\\" + skeletonSideProjection->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving air volume skeleton side projection to: " << path << std::endl;
	skeletonSideProjection->SaveCompressed(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;

	path = studyFolder + "\\" + skeletonFrontProjection->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving air volume skeleton front projection to: " << path << std::endl;
	skeletonFrontProjection->SaveCompressed(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;

	if (lastPipelineIndex < SimplifySkeletonIndex) return;

	// simplified skeleton
	//simplifiedSkeletonProjection->Resample();
	path = studyFolder + "\\" + simplifiedSkeletonSideProjection->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving air volume skeleton side projection to: " << path << std::endl;
	simplifiedSkeletonSideProjection->SaveCompressed(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;

	path = studyFolder + "\\" + simplifiedSkeletonFrontProjection->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving air volume skeleton front projection to: " << path << std::endl;
	simplifiedSkeletonFrontProjection->SaveCompressed(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;

	// minimal skeleton
	path = studyFolder + "\\" + minimalPathsSide->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving minimal paths side projection to: " << path << std::endl;
	minimalPathsSide->SaveCompressed(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;

	path = studyFolder + "\\" + minimalPathsFront->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving minimal paths front projection to: " << path << std::endl;
	minimalPathsFront->SaveCompressed(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;

	// cross section
	path = studyFolder + "\\" + sideCrossSection->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving cross sections side projection to: " << path << std::endl;
	sideCrossSection->Save(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;

	path = studyFolder + "\\" + frontCrossSection->GetFileName() + ".jpg";
	std::cout << "[AirVolume] Saving cross sections front projection to: " << path << std::endl;
	frontCrossSection->Save(path.c_str());
	std::cout << "[AirVolume] Saving done." << std::endl;
}


void AirVolumeAlgorithm::GetCrossSectionsProfile(double** arr, int* size)
{
	auto length = minimalCrossSections.size();
	*size = length;
	*arr = new double[length];
	for (int i = 0; i < length; i++)
		(*arr)[i] = minimalCrossSections[i];
}

void AirVolumeAlgorithm::GetCrossSectionsProfile2(double** arr, int* size)
{
	auto length = minimalCrossSections2.size();
	*size = length;
	*arr = new double[length];
	for (int i = 0; i < length; i++)
		(*arr)[i] = minimalCrossSections2[i];
}

void AirVolumeAlgorithm::GetCrossSectionsProfile3(double** arr, int* size)
{
	auto length = minimalCrossSections3.size();
	*size = length;
	*arr = new double[length];
	for (int i = 0; i < length; i++)
		(*arr)[i] = minimalCrossSections3[i];
}


void AirVolumeAlgorithm::GetCrossSectionsProfile4(double** arr, int* size)
{
	auto length = minimalCrossSections4.size();
	*size = length;
	*arr = new double[length];
	for (int i = 0; i < length; i++)
		(*arr)[i] = minimalCrossSections4[i];
}

void AirVolumeAlgorithm::GetCrossSectionCoord(int index, int** arr, int* size)
{
	auto length = minimalCrossSectionsCoords[index].size();
	*size = length;
	*arr = new int[length];
	for (int i = 0; i < length; i++)
		(*arr)[i] = minimalCrossSectionsCoords[index][i];
}

void AirVolumeAlgorithm::GetCrossSectionNormal(int index, double** arr, int* size)
{
	auto length = minimalCrossSectionsNormals[index].size();
	*size = length;
	*arr = new double[length];
	for (int i = 0; i < length; i++)
		(*arr)[i] = minimalCrossSectionsNormals[index][i];
}

std::vector<double> AirVolumeAlgorithm::CreateLine(DicomImageType::IndexType first, DicomImageType::IndexType second)
{
	std::vector<double> line = { 0, 0 };
	if (first[0] == second[0]) return line;

	line[0] = (double)(second[1] - first[1]) / (second[0] - first[0]);
	line[1] = second[1] - (double)(line[0] * second[0]);
	return line;
}

float AirVolumeAlgorithm::GetProgress()
{
	return progressAccumulator->GetAccumulatedProgress();
}

AirVolumeAlgorithm::~AirVolumeAlgorithm()
{
	std::cout << "AirVolumeAlgorithm dctor... ";
	
	progressAccumulator->UnregisterAllFilters();

	if (sideProjection != nullptr) {delete sideProjection; sideProjection = nullptr;}
	if (frontProjection != nullptr) {delete frontProjection; frontProjection = nullptr;}
	//if (lowerHalfSideProjection != nullptr) delete lowerHalfSideProjection;
	if (skeletonSideProjection != nullptr) {delete skeletonSideProjection; skeletonSideProjection = nullptr;}
	if (skeletonFrontProjection != nullptr) {delete skeletonFrontProjection; skeletonFrontProjection = nullptr;}
	if (simplifiedSkeletonSideProjection != nullptr) {delete simplifiedSkeletonSideProjection; simplifiedSkeletonSideProjection = nullptr;}
	if (simplifiedSkeletonFrontProjection != nullptr){ delete simplifiedSkeletonFrontProjection; simplifiedSkeletonFrontProjection = nullptr;}
	if (minimalPathsSide != nullptr) {delete minimalPathsSide; minimalPathsSide = nullptr;}
	if (minimalPathsFront != nullptr) {delete minimalPathsFront; minimalPathsFront = nullptr;}
	if (frontCrossSection != nullptr) {delete frontCrossSection; frontCrossSection = nullptr;}
	if (sideCrossSection != nullptr) {delete sideCrossSection; sideCrossSection = nullptr;}
	std::cout << "done." << std::endl;
}