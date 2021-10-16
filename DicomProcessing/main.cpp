#include "vtkRender.h"
#include "DicomProcessing.h"
#include "AirVolumeAlgorithm.h"
#include "MandibularCondylesAlgorithm.h"
#include "RamusContoursToRamusLineAlgorithm.h"
#include "ITK-5.1/itkVersion.h"
#include "common.h"
#include "consoleHelpers.h"

VtkRender* PrepareVtkRender(bool loadPresets = true)
{
	auto vtkRender = new VtkRender();
	if (loadPresets)
	{
		std::cout << "Loading presets..." << std::endl;
		Presets::GetInstance().LoadPresets();
		vtkRender->SetPresets(&(Presets::GetInstance()));
	}
	return vtkRender;
}

void runVtkTest()
{
	auto tester = new VtkRender();
	tester->LoadTestVolume();
}

void runAirVolumeProcessingTest(DicomSeries* dicomSeries, VtkRender* vtkRender)
{
	std::cout << "Processing air volume..." << std::endl;

	auto airVolumeDetector = new AirVolumeAlgorithm();
	airVolumeDetector->Run(dicomSeries);
	delete airVolumeDetector;

	std::cout << "Rendering..." << std::endl;

	//vtkRender->UpdateData(dicomSeries->GetVtkImageData(true));
	vtkRender->SetRenderOption(Volumetric);
	vtkRender->SetWindowSize(900, 600);
	vtkRender->LoadDicomVolume(dicomSeries);
	vtkRender->Render();
}

void runMandibularProcessingTest(DicomSeries* dicomSeries, VtkRender* vtkRender)
{
	std::cout << "Processing mandibular condyles..." << std::endl;

	auto algorithm = new MandibularCondylesAlgorithm();
	int result = algorithm->Run(dicomSeries);
	delete algorithm;

	if (result != 0) return;

	std::cout << "Rendering..." << std::endl;

	//vtkRender->UpdateData(dicomSeries->GetVtkImageData(true));
	vtkRender->SetRenderOption(Volumetric);
	vtkRender->SetWindowSize(900, 600);
	vtkRender->LoadDicomVolume(dicomSeries);
	vtkRender->Render();
}

void runSlicerTest(DicomSeries* dicomSeries, VtkRender* vtkRender)
{
	std::cout << "Rendering..." << std::endl;

	//vtkRender->UpdateData(dicomSeries->GetVtkImageData(true));
	vtkRender->SetRenderOption(FlatXY);
	vtkRender->SetWindowSize(900, 600);
	vtkRender->LoadDicomVolume(dicomSeries);
	vtkRender->Render();
}

DicomSeries* LoadDicomSeries()
{
	auto dicomSeries = new DicomSeries();
	bool isOpened = false;
	do
	{
		std::cout << "Specify dicom series folder path: " << std::endl << ">> ";
		std::string inputPath;
		const char* path = "C:\\Users\\pp\\Desktop\\Pawel - Dane\\MGR\\Dane\\Filmanowicz\\post";
		std::cin.clear();
		std::cin.ignore(1);
		std::getline(std::cin, inputPath);
		if (inputPath != "default")
			path = inputPath.c_str();

		ConsoleHelpers::writeStart();
		std::cout << "Loading series: " << path << std::endl;

		dicomSeries->InitializeReader();
		isOpened = dicomSeries->Open(path);
		//vtkRender->LoadDicomVolume(dicomSeries);
		//vtkRender->Render();

		if (!isOpened)
			std::cout << "Cannot open dicom series." << std::endl << std::endl;
		else
		{
			std::cout << "Patient name: " << dicomSeries->GetPatientName() << std::endl;
			std::cout << "Patient id: " << dicomSeries->GetPatientId() << std::endl;
			std::cout << "Study date: " << dicomSeries->GetStudyDate() << std::endl;

			using MinMaxCalcType = itk::MinimumMaximumImageCalculator<DicomSeriesType>;
			auto minMaxCalc = MinMaxCalcType::New();
			minMaxCalc->SetImage(dicomSeries->GetItkImageData());
			minMaxCalc->ComputeMaximum();
			minMaxCalc->ComputeMinimum();
			std::cout << "Max: " << minMaxCalc->GetMaximum() << std::endl;
			std::cout << "Min: " << minMaxCalc->GetMinimum() << std::endl;

			dicomSeries->GetItkImageData()->Print(std::cout);
		}
		ConsoleHelpers::writeEnd();

	} while (!isOpened);

	return dicomSeries;
}

DicomImage* LoadDicomImage(std::string & outputPath)
{
	auto dicomimage = new DicomImage();
	bool isOpened = false;
	do
	{
		std::cout << "Specify dicom image path: " << std::endl << ">> ";
		std::string inputPath;
		const char* path = "C:\\temp\\left.dcm";
		std::cin.clear();
		std::cin.ignore(1);
		std::getline(std::cin, inputPath);
		if (inputPath != "default" && inputPath != "ramusl" && inputPath != "ramusr" && inputPath != "notchl" && inputPath != "notchr")
			path = inputPath.c_str();

		if (inputPath == "ramusl")
			path = "C:\\temp\\left - contour.dcm";
		if (inputPath == "ramusr")
			path = "C:\\temp\\right - contour.dcm";

		ConsoleHelpers::writeStart();
		std::cout << "Loading series: " << path << std::endl;

		isOpened = dicomimage->Open(path);
		//vtkRender->LoadDicomVolume(dicomSeries);
		//vtkRender->Render();

		if (!isOpened)
			std::cout << "Cannot open dicom image." << std::endl << std::endl;
		else
		{
			std::cout << "Patient name: " << dicomimage->GetPatientName() << std::endl;
			std::cout << "Patient id: " << dicomimage->GetPatientId() << std::endl;
			std::cout << "Study date: " << dicomimage->GetStudyDate() << std::endl;
			std::cout << "File path: " << dicomimage->GetFilePath() << std::endl;
			std::cout << "File name: " << dicomimage->GetFileName() << std::endl;

			using MinMaxCalcType = itk::MinimumMaximumImageCalculator<DicomImageType>;
			auto minMaxCalc = MinMaxCalcType::New();
			minMaxCalc->SetImage(dicomimage->GetItkDicomImage());
			minMaxCalc->ComputeMaximum();
			minMaxCalc->ComputeMinimum();
			std::cout << "Max: " << minMaxCalc->GetMaximum() << std::endl;
			std::cout << "Min: " << minMaxCalc->GetMinimum() << std::endl;
			outputPath = "C:\\temp\\left - with spacing.dcm";

			dicomimage->GetItkDicomImage()->Print(std::cout);
		}
		ConsoleHelpers::writeEnd();

	} while (!isOpened);

	return dicomimage;
}

void saveDicomImageWithSpacing(DicomImage* dicomImage, std::string outputPath)
{
	dicomImage->Save(outputPath.c_str(), true);
}


void saveImageWithLine(DicomImage* image, std::vector<double> line)
{
	auto tolerance = line[0] < 1 ? 1 : line[0] < 5 ? 2 : line[0] < 20 ? 3 : 4;
	using IteratorType = itk::ImageRegionIteratorWithIndex<DicomImageType>;
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

void getRamusLine(DicomImage* dicomImage)
{
	auto ramusContours = RamusContoursToRamusLineAlgorithm::New();
	ramusContours->SetForegroundValue(SHRT_MAX);
	ramusContours->SetImage(dicomImage->GetItkDicomImage());
	ramusContours->Compute();
	auto line = ramusContours->GetRamusLine();
	saveImageWithLine(dicomImage, line);
}

int GetPathOption()
{
	std::cout << "Please specify file type: " << std::endl;
	std::cout << "1) Dicom series" << std::endl;
	std::cout << "2) Dicom image" << std::endl;
	std::cout << "3) Exit" << std::endl;
	std::cout << ">> ";

	int option = -1;
	while (option < 0 || option > 3)
	{
		std::cin >> option;
		if (option < 0 || option > 3)
			std::cout << "No type. Please specify proper type: " << std::endl << ">> ";
	}

	return option;
}

int GetOption(int pathOption)
{
	int option = -1;
	if (pathOption == 1)
	{
		std::cout << "Please specify action: " << std::endl;
		std::cout << "1) Air volume processing test" << std::endl;
		std::cout << "2) Mandibular condyle processing test" << std::endl;
		std::cout << "3) Slicer test" << std::endl;
		std::cout << "4) Exit" << std::endl;
		std::cout << ">> ";

		while (option < 0 || option > 4)
		{
			std::cin >> option;
			if (option < 0 || option > 4)
				std::cout << "No action. Please specify proper action: " << std::endl << ">> ";
		}
	}
	else if (pathOption == 2)
	{
		std::cout << "Please specify action: " << std::endl;
		std::cout << "1) Save resampled with spacing as a copy" << std::endl;
		std::cout << "2) Ramus line (requires ramus contours image)" << std::endl;
		std::cout << "3) Notch line (requires ramus contours image)" << std::endl;
		std::cout << "4) Exit" << std::endl;
		std::cout << ">> ";

		while (option < 0 || option > 4)
		{
			std::cin >> option;
			if (option < 0 || option > 4)
				std::cout << "No action. Please specify proper action: " << std::endl << ">> ";
		}
	}

	return option;
}

void svdTest();

int main()
{
	//std::cout << vtkVersion::GetVTKSourceVersion() << std::endl;
	svdTest();
	return 0;

	auto vtkRender = PrepareVtkRender();

	while (true)
	{
		auto pathOption = GetPathOption();

		if (pathOption == 1)
		{
			auto dicomSeries = LoadDicomSeries();

			auto option = GetOption(pathOption);
			ConsoleHelpers::writeStart();
			if (option == 1)
				runAirVolumeProcessingTest(dicomSeries, vtkRender);
			else if (option == 2)
				runMandibularProcessingTest(dicomSeries, vtkRender);
			else if (option == 3)
				runSlicerTest(dicomSeries, vtkRender);
			ConsoleHelpers::writeEnd();
		}
		else if (pathOption == 2)
		{
			std::string path;
			auto dicomImage = LoadDicomImage(path);

			auto option = GetOption(pathOption);
			ConsoleHelpers::writeStart();
			if (option == 1)
				saveDicomImageWithSpacing(dicomImage, path);
			else if (option == 2)
				getRamusLine(dicomImage);

			ConsoleHelpers::writeEnd();
		}
		else
			return 0;
	}

	return 0;
}


void svdTest()
{
	std::setprecision(4);

	double trgPoints[9] =
	{
		-59.434, -5.7607, -1.935,
		 63.6287, -4.3622, -0.06,
		 0.349513, -65.893398, 8.690002
	};
	double srcPoints[9] =
	{
		-57.269, -15.804, -8.685,
		 66.1076, -15.8042, -4.935,
		 2.828224, -76.60895, 2.565002
	};
	double trgCenter[3] = { 1.39384, -25.33875, 1.815002 };
	double srcCenter[3] = { 3.888772, -36.30813, -3.684998 };

	//double trgPoints[9] =
	//{
	//	-59.434, -5.7607, -1.935,
	//	 63.6287, -4.3622, -0.06,
	//	 0.349513, -65.893398, 8.690002
	//};
	//double srcPoints[9] =
	//{
	//	-57.806226, -22.649147, -9.309998,
	//	 65.396982, -20.793677, -10.559998,
	//	 3.795378, -82.024187, 4.440002
	//};
	//double trgCenter[3] = { 1.39384, -25.33875, 1.815002 };
	//double srcCenter[3] = { 3.795378, -41.946035, -5.559998 };

	//double trgPoints[9] =
	//{
	//	-64.77537, -11.273696, -38.059998,
	//	 65.74236, -10.629164, -29.309998,
	//	-3.222564, -62.836256, 9.440002,
	//};
	//double srcPoints[9] =
	//{
	//	-65.761728, -15.621518, -19.815002,
	//	 64.316352, -8.756286, -24.190002,
	//	 3.613248, -66.207438, 18.309998
	//};
	//double trgCenter[3] = { -0.966702, -28.353794, -19.309998 };
	//double srcCenter[3] = { 0.722624, -30.435966, -8.565002 };

	//double trgPoints[9] =
	//{
	//	1, 0, 0,
	//	0, 1, 0,
	//	0, 0, 1 
	//};
	//double srcPoints[9] =
	//{
	//	1.866, 2, 0.5,
	//	1, 3, 1,
	//	1.5, 2, 1.866
	//};
	//double trgCenter[3] = { 0, 0, 0 };
	//double srcCenter[3] = { 1, 2, 1 };

	auto pointsCount = 3;
	// with 4 points
	//auto pointsCount = 4;

	//double trgPoints[12] =
	//{
	//	-59.434, -5.7607, -1.935,
	//	 63.6287, -4.3622, -0.06,
	//	 0.349513, -65.893398, 8.690002,
	//	 1.39834, -75.68245, -8.184998
	//};
	//double srcPoints[12] =
	//{
	//	-57.806226, -22.649147, -9.309998,
	//	 65.396982, -20.793677, -10.559998,
	//	 3.795378, -82.024187, 4.440002,
	//	 4.166472, -92.785913, -14.309998
	//};
	//double trgCenter[3] = { 1.864485333, -28.60177133, -3.393331333 };
	//double srcCenter[3] = { 3.919076, -45.409579, -11.39333133 };

	//double trgPoints[12] =
	//{
	//	-64.77537, -11.273696, -38.059998,
	//	 65.74236, -10.629164, -29.309998,
	//	-3.222564, -62.836256, 9.440002,
	//	2.578224, -83.46128, -7.434998
	//};
	//double srcPoints[12] =
	//{
	//	-65.761728, -15.621518, -19.815002,
	//	 64.316352, -8.756286, -24.190002,
	//	 3.613248, -66.207438, 18.309998,
	//	 3.613248, -85.71915, -1.065002
	//};
	//double trgCenter[3] = { 1.181738, -35.12138, -24.934998 };
	//double srcCenter[3] = { 0.722624, -36.69898467, -15.02333533 };

	auto transform = DicomProcessing::CalculateTransformationMatrix(srcPoints, srcCenter, trgPoints, trgCenter, pointsCount);

	// check
	double calculated[3][3] =
	{
		{
			srcPoints[0] * transform[0] + srcPoints[1] * transform[1] + srcPoints[2] * transform[2] + transform[3],
			srcPoints[0] * transform[4] + srcPoints[1] * transform[5] + srcPoints[2] * transform[6] + transform[7],
			srcPoints[0] * transform[8] + srcPoints[1] * transform[9] + srcPoints[2] * transform[10] + transform[11]
		},
		{
			srcPoints[3] * transform[0] + srcPoints[4] * transform[1] + srcPoints[5] * transform[2] + transform[3],
			srcPoints[3] * transform[4] + srcPoints[4] * transform[5] + srcPoints[5] * transform[6] + transform[7],
			srcPoints[3] * transform[8] + srcPoints[4] * transform[9] + srcPoints[5] * transform[10] + transform[11]
		},																 
		{
			srcPoints[6] * transform[0] + srcPoints[7] * transform[1] + srcPoints[8] * transform[2] + transform[3],
			srcPoints[6] * transform[4] + srcPoints[7] * transform[5] + srcPoints[8] * transform[6] + transform[7],
			srcPoints[6] * transform[8] + srcPoints[7] * transform[9] + srcPoints[8] * transform[10] + transform[11]
		}																												
	};

	std::cout << "Sources: " << std::endl;
	std::cout << srcPoints[0] << "," << srcPoints[1] << "," << srcPoints[2] << std::endl;
	std::cout << srcPoints[3] << "," << srcPoints[4] << "," << srcPoints[5] << std::endl;
	std::cout << srcPoints[6] << "," << srcPoints[7] << "," << srcPoints[8] << std::endl << std::endl;

	std::cout << "Targets: " << std::endl;
	std::cout << trgPoints[0] << "," << trgPoints[1] << "," << trgPoints[2] << std::endl;
	std::cout << trgPoints[3] << "," << trgPoints[4] << "," << trgPoints[5] << std::endl;
	std::cout << trgPoints[6] << "," << trgPoints[7] << "," << trgPoints[8] << std::endl << std::endl;

	std::cout << "Transform: " << std::endl;
	std::cout << transform[0] << "," << transform[1] << "," << transform[2] << "," << transform[3] << std::endl;
	std::cout << transform[4] << "," << transform[5] << "," << transform[6] << "," << transform[7] << std::endl;
	std::cout << transform[8] << "," << transform[9] << "," << transform[10] << "," << transform[11] << std::endl;
	std::cout << transform[12] << "," << transform[13] << "," << transform[14] << "," << transform[15] << std::endl << std::endl;

	std::cout << "Results: " << std::endl;
	std::cout << calculated[0][0] << "," << calculated[0][1] << "," << calculated[0][2] << std::endl;
	std::cout << calculated[1][0] << "," << calculated[1][1] << "," << calculated[1][2] << std::endl;
	std::cout << calculated[2][0] << "," << calculated[2][1] << "," << calculated[2][2] << std::endl << std::endl;

	std::cout << "Diff: " << std::endl;
	std::cout << std::sqrt(std::pow(calculated[0][0] - trgPoints[0], 2) + std::pow(calculated[0][1] - trgPoints[1], 2) + std::pow(calculated[0][2] - trgPoints[2], 2)) << std::endl;
	std::cout << std::sqrt(std::pow(calculated[1][0] - trgPoints[3], 2) + std::pow(calculated[1][1] - trgPoints[4], 2) + std::pow(calculated[1][2] - trgPoints[5], 2)) << std::endl;
	std::cout << std::sqrt(std::pow(calculated[2][0] - trgPoints[6], 2) + std::pow(calculated[2][1] - trgPoints[7], 2) + std::pow(calculated[2][2] - trgPoints[8], 2)) << std::endl;

	double calculatedBack[3][3] =
	{
		{
			calculated[0][0] * transform[0] + calculated[0][1] * transform[4] + calculated[0][2] * transform[8] - transform[3],
			calculated[0][0] * transform[1] + calculated[0][1] * transform[5] + calculated[0][2] * transform[9] - transform[7],
			calculated[0][0] * transform[2] + calculated[0][1] * transform[6] + calculated[0][2] * transform[10] - transform[11]
		},
		{
			calculated[1][0] * transform[0] + calculated[1][1] * transform[4] + calculated[1][2] * transform[8] - transform[3],
			calculated[1][0] * transform[1] + calculated[1][1] * transform[5] + calculated[1][2] * transform[9] - transform[7],
			calculated[1][0] * transform[2] + calculated[1][1] * transform[6] + calculated[1][2] * transform[10] - transform[11]
		},
		{
			calculated[2][0] * transform[0] + calculated[2][1] * transform[4] + calculated[2][2] * transform[8] - transform[3],
			calculated[2][0] * transform[1] + calculated[2][1] * transform[5] + calculated[2][2] * transform[9] - transform[7],
			calculated[2][0] * transform[2] + calculated[2][1] * transform[6] + calculated[2][2] * transform[10] - transform[11]
		}
	};

	std::cout << "Sources: " << std::endl;
	std::cout << srcPoints[0] << "," << srcPoints[1] << "," << srcPoints[2] << std::endl;
	std::cout << srcPoints[3] << "," << srcPoints[4] << "," << srcPoints[5] << std::endl;
	std::cout << srcPoints[6] << "," << srcPoints[7] << "," << srcPoints[8] << std::endl << std::endl;

	std::cout << "Transformed back: " << std::endl;
	std::cout << calculatedBack[0][0] << "," << calculatedBack[0][1] << "," << calculatedBack[0][2] << std::endl;
	std::cout << calculatedBack[1][0] << "," << calculatedBack[1][1] << "," << calculatedBack[1][2] << std::endl;
	std::cout << calculatedBack[2][0] << "," << calculatedBack[2][1] << "," << calculatedBack[2][2] << std::endl << std::endl;

}