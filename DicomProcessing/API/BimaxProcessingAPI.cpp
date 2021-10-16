#include "BimaxProcessingAPI.h"
#include "helpers.h"

///////////////////////////////////////////////////////
// Reading image
///////////////////////////////////////////////////////
DLLEXPORT RgbImage* Image_CreateInstance()
{
	return new RgbImage();
}
DLLEXPORT void DicomSeries_InitializeReader(DicomSeries* instance)
{
	instance->InitializeReader();
}
DLLEXPORT void Image_Open(RgbImage* instance, const char* imagePath)
{
	instance->Open(imagePath);
}
DLLEXPORT void DicomImage_Save(DicomImage* instance, const char* imagePath)
{
	instance->Save(imagePath);
}
DLLEXPORT BYTE* Image_GetData(RgbImage* instance)
{
	return instance->GetData();
}
DLLEXPORT int Image_GetWidth(RgbImage* instance)
{
	return instance->GetWidth();
}
DLLEXPORT int Image_GetHeight(RgbImage* instance)
{
	return instance->GetHeight();
}
DLLEXPORT void Image_Dispose(RgbImage* instance)
{
	delete instance;
}

///////////////////////////////////////////////////////
// Reading single dicom
///////////////////////////////////////////////////////
DLLEXPORT DicomImage * DicomImage_CreateInstance()
{
	return new DicomImage();
}
DLLEXPORT void DicomImage_Open(DicomImage* instance, const char* filePath)
{
	instance->Open(filePath);
}
DLLEXPORT signed short * DicomImage_GetData(DicomImage * instance)
{
	return instance->GetData();
}
DLLEXPORT BYTE * DicomImage_GetImageBytes(DicomImage * instance)
{
	return instance->GetImageBytes();
}
DLLEXPORT int DicomImage_GetWidth(DicomImage * instance)
{
	return instance->GetWidth();
}
DLLEXPORT int DicomImage_GetHeight(DicomImage * instance)
{
	return instance->GetHeight();
}
DLLEXPORT void DicomImage_Dispose(DicomImage * instance)
{
	delete instance;
}

///////////////////////////////////////////////////////
// Reading dicom series
///////////////////////////////////////////////////////
DLLEXPORT DicomSeries * DicomSeries_CreateInstance()
{
	return new DicomSeries();
}
DLLEXPORT bool DicomSeries_Open(DicomSeries * instance, const char * folderPath)
{
	return instance->Open(folderPath);
}
DLLEXPORT void DicomSeries_Save(DicomSeries* instance, const char* folderPath)
{
	instance->Save(folderPath);
}
DLLEXPORT double DicomSeries_GetProgress(DicomSeries * instance)
{
	return instance->GetReaderProgress();
}
DLLEXPORT char * DicomSeries_GetPatientName(DicomSeries* instance)
{
	return helpers::StringToCStr(instance->GetPatientName());
}
DLLEXPORT char * DicomSeries_GetPatientId(DicomSeries* instance)
{
	return helpers::StringToCStr(instance->GetPatientId());
}
DLLEXPORT char* DicomSeries_GetImageId(DicomSeries* instance)
{
	return helpers::StringToCStr(instance->GetImageId());
}
DLLEXPORT char * DicomSeries_GetStudyDate(DicomSeries* instance)
{
	return helpers::StringToCStr(instance->GetStudyDate());
}
DLLEXPORT char * DicomSeries_GetAcquisitionDate(DicomSeries* instance)
{
	return helpers::StringToCStr(instance->GetAcquisitionDate());
}
DLLEXPORT void DicomSeries_DisposeReader(DicomSeries* instance)
{
	instance->DisposeReader();
}
DLLEXPORT void DicomSeries_Dispose(DicomSeries* instance)
{
	instance->~DicomSeries();
	delete instance;
}

///////////////////////////////////////////////////////
// Rendering with VTK
///////////////////////////////////////////////////////
DLLEXPORT VtkRender* VtkRender_CreateInstance()
{
	return new VtkRender();
}
DLLEXPORT void VtkRender_SetRenderHost(VtkRender* instance, HWND handle)
{
	instance->SetRenderHost(handle);
}
DLLEXPORT bool VtkRender_LoadTestVolume(VtkRender* instance, int width, int height)
{
	return instance->LoadTestVolume();
}
DLLEXPORT bool VtkRender_LoadDicomVolume(VtkRender* instance, DicomSeries* dicomSeries, int width, int height)
{
	return instance->LoadDicomVolume(dicomSeries);
}
DLLEXPORT bool VtkRender_LoadAdditionalDicomVolume(VtkRender* instance, DicomSeries* dicomSeries)
{
	return instance->LoadAdditionalDicomVolume(dicomSeries);
}
DLLEXPORT void VtkRender_SetWindowSize(VtkRender* instance, int width, int height)
{
	instance->SetWindowSize(width, height);
}
DLLEXPORT void VtkRender_Render(VtkRender* instance)
{
	instance->Render();
}
//DLLEXPORT void VtkRender_StartRender(VtkRender* instance)
//{
//	instance->StartRender();
//}
DLLEXPORT void VtkRender_Dispose(VtkRender* instance)
{
	delete instance;
}
DLLEXPORT void VtkRender_SetPreset(VtkRender* instance, Preset* preset)
{
	instance->SetVolumeProperty(preset);
}
DLLEXPORT void VtkRender_SetPresetsSource(VtkRender* instance, Presets* source)
{
	instance->SetPresets(source);
}
DLLEXPORT void VtkRender_UpdateData(VtkRender* instance, DicomSeries* series)
{
	instance->UpdateData(series->GetVtkImageData(true));
}
DLLEXPORT void VtkRender_SetRenderOption(VtkRender* instance, int option)
{
	instance->SetRenderOption(static_cast<RenderType>(option));
}
DLLEXPORT void VtkRender_SetSliceIndex(VtkRender* instance, int index)
{
	instance->SetSliceIndex(index);
}
DLLEXPORT int VtkRender_GetSliceIndex(VtkRender* instance)
{
	return instance->GetSliceIndex();
}
DLLEXPORT int VtkRender_GetMaxSliceIndex(VtkRender* instance)
{
	return instance->GetMaxSliceIndex();
}
DLLEXPORT VtkRenderSync* VtkRenderSync_CreateInstance()
{
	return new VtkRenderSync();
}
DLLEXPORT void VtkRenderSync_RegisterReslicer(VtkRenderSync* instance, VtkRender* render)
{
	auto rw = render->GetReslicer();
	instance->RegisterReslicer(rw);
}
DLLEXPORT void VtkRender_ToggleResliceCursors(VtkRender* instance, bool value)
{
	instance->ToggleResliceCursors(value);
}
DLLEXPORT void VtkRender_SetRenderSync(VtkRender* instance, VtkRenderSync* sync)
{
	instance->SetRenderSync(sync);
}
DLLEXPORT void VtkRenderSync_SetResliceCenter(VtkRenderSync* instance, double x, double y, double z)
{
	instance->SetResliceCenter(x, y, z);
}
DLLEXPORT void VtkRenderSync_SetResliceNormal(VtkRenderSync* instance, double x, double y, double z)
{
	instance->SetResliceNormal(x, y, z);
}
DLLEXPORT void VtkRenderSync_ResetReslicers(VtkRenderSync* instance)
{
	instance->ResetReslicers();
}
DLLEXPORT void VtkRender_SetAdditionalVolumeTransform(VtkRender* instance, double* transform)
{
	instance->SetAdditionalVolumeTransform(transform);
}
DLLEXPORT void VtkRender_ToggleShowSecondVolume(VtkRender* instance, bool value)
{
	instance->ToggleShowSecondVolume(value);
}
DLLEXPORT void VtkRender_SelectPoint(VtkRender* instance, double x, double y, double z, bool onFirst)
{
	instance->SelectPoint(x, y, z, onFirst);
}
DLLEXPORT void VtkRender_ClearSelectedPoints(VtkRender* instance, bool onFirst)
{
	instance->ClearPointsSelection(onFirst);
}


///////////////////////////////////////////////////////
// Presets
///////////////////////////////////////////////////////
DLLEXPORT Presets* Presets_GetInstance()
{
	return &Presets::GetInstance();
}
DLLEXPORT Preset* Presets_CreatePreset(Presets* instance)
{
	return instance->CreatePreset();
}
DLLEXPORT void Preset_SetName(Preset* instance, char* name)
{
	instance->SetName(name);
}
DLLEXPORT void Preset_SetValues(Preset* instance, double specularPower,
	double specular, double shade, double ambient, double diffuse)
{
	instance->SetProperties(specularPower, specular, shade, ambient, diffuse);
}
DLLEXPORT void Preset_AddGradientPoint(Preset* instance, double x, double y)
{
	instance->AddGradientPoint(x, y);
}
DLLEXPORT void Preset_AddScalarPoint(Preset* instance, double x, double y)
{
	instance->AddScalarPoint(x, y);
}
DLLEXPORT void Preset_AddColorTransfer(Preset* instance, double x, double r,
	double g, double b)
{
	instance->AddColorTransfer(x, r, g, b);
}

///////////////////////////////////////////////////////
// Processing
///////////////////////////////////////////////////////
DLLEXPORT void DicomProcessing_ThresholdBetween(DicomSeries* instance, short lower, short upper)
{
	DicomProcessing::ThresholdBetween(instance, lower, upper);
}

DLLEXPORT AirVolumeAlgorithm* AirVolumeAlgorithm_CreateInstance()
{
	return new AirVolumeAlgorithm();
}
DLLEXPORT void AirVolumeAlgorithm_Dispose(AirVolumeAlgorithm* instance)
{
	delete instance;
}
DLLEXPORT void AirVolumeAlgorithm_Run(AirVolumeAlgorithm* instance, DicomSeries* dicomSeries)
{
	instance->Run(dicomSeries);
}
DLLEXPORT long AirVolumeAlgorithm_GetVoxelCount(AirVolumeAlgorithm* instance)
{
	return instance->GetVoxelCount();
}
DLLEXPORT double AirVolumeAlgorithm_GetCalculatedVolume(AirVolumeAlgorithm* instance)
{
	return instance->GetCalculatedVolume();
}
DLLEXPORT float AirVolumeAlgorithm_GetProgress(AirVolumeAlgorithm* instance)
{
	return instance->GetProgress();
}
DLLEXPORT void AirVolumeAlgorithm_DumpIntermediates(AirVolumeAlgorithm* instance, const char* folder)
{
	instance->DumpIntermediates(folder);
}
DLLEXPORT double AirVolumeAlgorithm_GetMinimalCrossSection(AirVolumeAlgorithm* instance)
{
	return instance->GetMinimalCrossSection();
}

DLLEXPORT double AirVolumeAlgorithm_GetMinimalCrossSection2(AirVolumeAlgorithm* instance)
{
	return instance->GetMinimalCrossSection2();
}
DLLEXPORT double AirVolumeAlgorithm_GetMinimalCrossSection3(AirVolumeAlgorithm* instance)
{
	return instance->GetMinimalCrossSection3();
}
DLLEXPORT double AirVolumeAlgorithm_GetMinimalCrossSection4(AirVolumeAlgorithm* instance)
{
	return instance->GetMinimalCrossSection4();
}
DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSections2(
	AirVolumeAlgorithm* instance, double** arr, int* size)
{
	instance->GetCrossSectionsProfile2(arr, size);
}
DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSections(
	AirVolumeAlgorithm* instance, double** arr, int* size)
{
	instance->GetCrossSectionsProfile(arr, size);
}
DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSections3(
	AirVolumeAlgorithm* instance, double** arr, int* size)
{
	instance->GetCrossSectionsProfile3(arr, size);
}
DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSections4(
	AirVolumeAlgorithm* instance, double** arr, int* size)
{
	instance->GetCrossSectionsProfile4(arr, size);
}
DLLEXPORT double AirVolumeAlgorithm_GetCalculatedPharynx(AirVolumeAlgorithm* instance)
{
	return instance->GetCalculatedPharynx();
}
DLLEXPORT double AirVolumeAlgorithm_GetCalculatedNasalCavity(AirVolumeAlgorithm* instance)
{
	return instance->GetCalculatedNasalCavity();
}
DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSectionCoord(AirVolumeAlgorithm* instance, int i, int** arr, int* size)
{
	instance->GetCrossSectionCoord(i, arr, size);
}
DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSectionNormal(AirVolumeAlgorithm* instance, int i, double** arr, int* size)
{
	instance->GetCrossSectionNormal(i, arr, size);
}

DLLEXPORT MandibularCondylesAlgorithm* MandibularCondylesAlgorithm_CreateInstance()
{
	return new MandibularCondylesAlgorithm();
}
DLLEXPORT MandibularGeneralThresholdAlgorithm* MandibularGeneralThresholdAlgorithm_CreateInstance()
{
	return new MandibularGeneralThresholdAlgorithm();
}
DLLEXPORT MandibularTeethThresholdAlgorithm* MandibularTeethThresholdAlgorithm_CreateInstance()
{
	return new MandibularTeethThresholdAlgorithm();
}
DLLEXPORT MandibularTeethDilateAlgorithm* MandibularTeethDilateAlgorithm_CreateInstance()
{
	return new MandibularTeethDilateAlgorithm();
}
DLLEXPORT MandibularSubtractAlgorithm* MandibularSubtractAlgorithm_CreateInstance()
{
	return new MandibularSubtractAlgorithm();
}
DLLEXPORT MandibularExtractionAlgorithm* MandibularExtractionAlgorithm_CreateInstance()
{
	return new MandibularExtractionAlgorithm();
}
DLLEXPORT UpperSkullExtractionAlgorithm* UpperSkullExtractionAlgorithm_CreateInstance()
{
	return new UpperSkullExtractionAlgorithm();
}
DLLEXPORT ZygomaticArchsExtractionAlgorithm* ZygomaticArchsExtractionAlgorithm_CreateInstance()
{
	return new ZygomaticArchsExtractionAlgorithm();
}

DLLEXPORT int MandibularBaseAlgorithm_Run(MandibularBaseAlgorithm* instance, DicomSeries* dicomSeries)
{
	return instance->Run(dicomSeries);
}
DLLEXPORT bool MandibularBaseAlgorithm_GetSuccess(MandibularBaseAlgorithm* instance)
{
	return instance->GetSuccess();
}
DLLEXPORT char* MandibularBaseAlgorithm_GetErrorMessage(MandibularBaseAlgorithm* instance)
{
	return helpers::StringToCStr(instance->GetErrorMessage());
}
DLLEXPORT float MandibularBaseAlgorithm_GetProgress(MandibularBaseAlgorithm* instance)
{
	return instance->GetProgress();
}

DLLEXPORT void MandibularCondylesAlgorithm_GetLeftCondylePosition(MandibularCondylesAlgorithm* instance, double** arr1, int* size1)
{
	instance->GetLeftCondylePositionEX(arr1, size1);
}
DLLEXPORT void MandibularCondylesAlgorithm_GetRightCondylePosition(MandibularCondylesAlgorithm* instance, double** arr2, int* size2)
{
	instance->GetRightCondylePositionEX(arr2, size2);
}
DLLEXPORT void MandibularCondylesAlgorithm_GetCondylesPositionDifference(MandibularCondylesAlgorithm* instance, double** arr3, int* size3)
{
	instance->GetCondylesDifferenceEX(arr3, size3);
}
DLLEXPORT double MandibularCondylesAlgorithm_GetCondylesDistance(MandibularCondylesAlgorithm* instance)
{
	return instance->GetCondylesDistance();
}
DLLEXPORT double MandibularCondylesAlgorithm_GetCondylesDistancePx(MandibularCondylesAlgorithm* instance)
{
	return instance->GetCondylesDistancePx();
}
DLLEXPORT void MandibularCondylesAlgorithm_GetNasionPoint(MandibularCondylesAlgorithm* instance, double** arr3, int* size3)
{
	instance->GetNasionPointEX(arr3, size3);
}

DLLEXPORT void MandibularCondylesAlgorithm_GetLeft2NasionDifference(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetLeft2NasionDifferenceEx(arr, size);
}
DLLEXPORT void MandibularCondylesAlgorithm_GetRight2NasionDifference(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetRight2NasionDifferenceEx(arr, size);
}
DLLEXPORT double MandibularCondylesAlgorithm_GetLeft2NasionDistancePx(MandibularCondylesAlgorithm* instance)
{
	return instance->GetLeft2NasionDistancePx();
}
DLLEXPORT double MandibularCondylesAlgorithm_GetLeft2NasionDistance(MandibularCondylesAlgorithm* instance)
{
	return instance->GetLeft2NasionDistance();
}
DLLEXPORT double MandibularCondylesAlgorithm_GetRight2NasionDistancePx(MandibularCondylesAlgorithm* instance)
{
	return instance->GetRight2NasionDistancePx();
}
DLLEXPORT double MandibularCondylesAlgorithm_GetRight2NasionDistance(MandibularCondylesAlgorithm* instance)
{
	return instance->GetRight2NasionDistance();
}
DLLEXPORT void MandibularCondylesAlgorithm_DumpIntermediates(MandibularCondylesAlgorithm* instance, const char* folder)
{
	instance->DumpIntermediates(folder);
}


DLLEXPORT void MandibularCondylesAlgorithm_GetLeftZygomaticPoint(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetLeftZygomaticPointEx(arr, size);
}
DLLEXPORT void MandibularCondylesAlgorithm_GetRightZygomaticPoint(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetRightZygomaticPointEx(arr, size);
}
DLLEXPORT void MandibularCondylesAlgorithm_GetCenter(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetCenter(arr, size);
}
DLLEXPORT void MandibularCondylesAlgorithm_GetLeftToCenterDifferenceEx(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetLeftToCenterDifferenceEx(arr, size);
}
DLLEXPORT void MandibularCondylesAlgorithm_GetRightToCenterDifferenceEx(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetRightToCenterDifferenceEx(arr, size);
}
DLLEXPORT double MandibularCondylesAlgorithm_GetLeftToCenterDistance(MandibularCondylesAlgorithm* instance)
{
	return instance->GetLeftToCenterDistance();
}
DLLEXPORT double MandibularCondylesAlgorithm_GetRightToCenterDistance(MandibularCondylesAlgorithm* instance)
{
	return instance->GetRightToCenterDistance();
}
DLLEXPORT void MandibularCondylesAlgorithm_GetImageSpacing(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetImageSpacing(arr, size);
}
DLLEXPORT void MandibularCondylesAlgorithm_GetImageOrigin(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetImageOrigin(arr, size);
}	
DLLEXPORT void MandibularCondylesAlgorithm_GetLeftCondyleReal(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetLeftCondyleFinal(arr, size);
}
DLLEXPORT void MandibularCondylesAlgorithm_GetRightCondyleReal(MandibularCondylesAlgorithm* instance, double** arr, int* size)
{
	instance->GetRightCondyleFinal(arr, size);
}



DLLEXPORT double* DicomProcessing_GetTransform(double* srcPoints, double* srcCenter, double* trgPoints, double* trgCenter)
{
	return DicomProcessing::CalculateTransformationMatrix(srcPoints, srcCenter, trgPoints, trgCenter);
}