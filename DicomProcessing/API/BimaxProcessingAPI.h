#pragma once

#include <iostream>
#include "RgbImage.h"
#include "DicomImage.h"
#include "vtk-9.0/vtkRender.h"
#include "Presets.h"
#include "DicomProcessing.h"
#include "AirVolumeAlgorithm.h"
#include "MandibularBaseAlgorithm.h"
#include "MandibularCondylesAlgorithm.h"
#include "MandibularPartialAlgorithms.h"

#define DLLEXPORT _declspec(dllexport)

extern "C"
{
	// reading image
	DLLEXPORT RgbImage* Image_CreateInstance();
	DLLEXPORT void Image_Open(RgbImage* instance, const char* imagePath);
	DLLEXPORT BYTE* Image_GetData(RgbImage* instance);
	DLLEXPORT int Image_GetWidth(RgbImage* instance);
	DLLEXPORT int Image_GetHeight(RgbImage* instance);
	DLLEXPORT void Image_Dispose(RgbImage* instance);

	// reading single dicom
	DLLEXPORT DicomImage* DicomImage_CreateInstance();
	DLLEXPORT void DicomImage_Open(DicomImage* instance, const char* imagePath);
	DLLEXPORT void DicomImage_Save(DicomImage* instance, const char* imagePath);
	DLLEXPORT signed short* DicomImage_GetData(DicomImage* instance);
	DLLEXPORT BYTE* DicomImage_GetImageBytes(DicomImage* instance);
	DLLEXPORT int DicomImage_GetWidth(DicomImage* instance);
	DLLEXPORT int DicomImage_GetHeight(DicomImage* instance);
	DLLEXPORT void DicomImage_Dispose(DicomImage* instance);

	// reading dicom series
	DLLEXPORT DicomSeries* DicomSeries_CreateInstance();
	DLLEXPORT void DicomSeries_InitializeReader(DicomSeries* instance);
	DLLEXPORT bool DicomSeries_Open(DicomSeries* instance, const char* folderPath);
	DLLEXPORT void DicomSeries_Save(DicomSeries* instance, const char* folderPath);
	//DLLEXPORT signed short * DicomSeries_GetDataBuffer(DicomSeries* instance);
	//DLLEXPORT BYTE* DicomSeries_GetImagesBuffer(DicomSeries* instance);
	//DLLEXPORT int DicomSeries_GetWidth(DicomSeries* instance);
	//DLLEXPORT int DicomSeries_GetHeight(DicomSeries* instance);
	//DLLEXPORT int DicomSeries_GetLayers(DicomSeries* instance);
	DLLEXPORT double DicomSeries_GetProgress(DicomSeries* instance);
	DLLEXPORT char* DicomSeries_GetPatientName(DicomSeries* instance);
	DLLEXPORT char* DicomSeries_GetPatientId(DicomSeries* instance);
	DLLEXPORT char* DicomSeries_GetImageId(DicomSeries* instance);
	DLLEXPORT char* DicomSeries_GetStudyDate(DicomSeries* instance);
	DLLEXPORT char* DicomSeries_GetAcquisitionDate(DicomSeries* instance);
	DLLEXPORT void DicomSeries_DisposeReader(DicomSeries* instance);
	DLLEXPORT void DicomSeries_Dispose(DicomSeries* instance);

	DLLEXPORT VtkRender* VtkRender_CreateInstance();
	DLLEXPORT void VtkRender_SetRenderHost(VtkRender* instance, HWND handle);
	DLLEXPORT bool VtkRender_LoadTestVolume(VtkRender* instance, int width, int height);
	DLLEXPORT bool VtkRender_LoadDicomVolume(VtkRender* instance, DicomSeries* dicomSeries, int width = 500, int height = 500);
	DLLEXPORT bool VtkRender_LoadAdditionalDicomVolume(VtkRender* instance, DicomSeries* dicomSeries);
	DLLEXPORT void VtkRender_Render(VtkRender* instance);
	DLLEXPORT void VtkRender_SetWindowSize(VtkRender* instance, int width, int height);
	DLLEXPORT void VtkRender_Dispose(VtkRender* instance);
	DLLEXPORT void VtkRender_SetPreset(VtkRender* instance, Preset* preset);
	DLLEXPORT void VtkRender_SetPresetsSource(VtkRender* instance, Presets* source);
	DLLEXPORT void VtkRender_UpdateData(VtkRender* instance, DicomSeries* series);
	DLLEXPORT void VtkRender_SetRenderOption(VtkRender* instance, int option);
	DLLEXPORT void VtkRender_SetSliceIndex(VtkRender* instance, int index);
	DLLEXPORT int VtkRender_GetSliceIndex(VtkRender* instance);
	DLLEXPORT int VtkRender_GetMaxSliceIndex(VtkRender* instance);
	DLLEXPORT VtkRenderSync* VtkRenderSync_CreateInstance();
	DLLEXPORT void VtkRenderSync_Cleanup(VtkRenderSync* instance) { instance->Cleanup(); }
	DLLEXPORT void VtkRenderSync_Dispose(VtkRenderSync* instance) { delete instance; }
	DLLEXPORT void VtkRenderSync_RegisterReslicer(VtkRenderSync* instance, VtkRender* render);
	DLLEXPORT void VtkRender_SetRenderSync(VtkRender* instance, VtkRenderSync* sync);
	DLLEXPORT void VtkRender_ToggleResliceCursors(VtkRender* instance, bool value);
	DLLEXPORT void VtkRenderSync_SetResliceCenter(VtkRenderSync* instance, double x, double y, double z);
	DLLEXPORT void VtkRenderSync_SetResliceNormal(VtkRenderSync* instance, double x, double y, double z);
	DLLEXPORT void VtkRenderSync_ResetReslicers(VtkRenderSync* instance);
	DLLEXPORT void VtkRender_SetAdditionalVolumeTransform(VtkRender* instance, double* transform);
	DLLEXPORT void VtkRender_ToggleShowSecondVolume(VtkRender* instance, bool value);
	DLLEXPORT void VtkRender_SelectPoint(VtkRender* instance, double x, double y, double z, bool onFirst);
	DLLEXPORT void VtkRender_ClearSelectedPoints(VtkRender* instance, bool onFirst);

	DLLEXPORT Presets* Presets_GetInstance();
	DLLEXPORT Preset* Presets_CreatePreset(Presets* instance);
	DLLEXPORT void Preset_SetName(Preset*, char*);
	DLLEXPORT void Preset_SetValues(Preset* instance, double specularPower, double specular, double shade, double ambient, double diffuse);
	DLLEXPORT void Preset_AddGradientPoint(Preset*, double, double);
	DLLEXPORT void Preset_AddScalarPoint(Preset*, double, double);
	DLLEXPORT void Preset_AddColorTransfer(Preset*, double, double, double, double);

	DLLEXPORT void DicomProcessing_ThresholdBetween(DicomSeries*, short, short);
	DLLEXPORT AirVolumeAlgorithm* AirVolumeAlgorithm_CreateInstance();
	DLLEXPORT void AirVolumeAlgorithm_Dispose(AirVolumeAlgorithm*);
	DLLEXPORT void AirVolumeAlgorithm_Run(AirVolumeAlgorithm*, DicomSeries*);
	DLLEXPORT long AirVolumeAlgorithm_GetVoxelCount(AirVolumeAlgorithm*);
	DLLEXPORT double AirVolumeAlgorithm_GetCalculatedVolume(AirVolumeAlgorithm*);
	DLLEXPORT double AirVolumeAlgorithm_GetMinimalCrossSection(AirVolumeAlgorithm*);
	DLLEXPORT double AirVolumeAlgorithm_GetMinimalCrossSection2(AirVolumeAlgorithm*);
	DLLEXPORT double AirVolumeAlgorithm_GetMinimalCrossSection3(AirVolumeAlgorithm*);
	DLLEXPORT double AirVolumeAlgorithm_GetMinimalCrossSection4(AirVolumeAlgorithm*);
	DLLEXPORT float AirVolumeAlgorithm_GetProgress(AirVolumeAlgorithm*);
	DLLEXPORT void AirVolumeAlgorithm_DumpIntermediates(AirVolumeAlgorithm*, const char*);
	DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSections2(AirVolumeAlgorithm*, double**, int*);
	DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSections(AirVolumeAlgorithm*, double**, int*);
	DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSections3(AirVolumeAlgorithm*, double**, int*);
	DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSections4(AirVolumeAlgorithm*, double**, int*);
	DLLEXPORT double AirVolumeAlgorithm_GetCalculatedPharynx(AirVolumeAlgorithm* instance);
	DLLEXPORT double AirVolumeAlgorithm_GetCalculatedNasalCavity(AirVolumeAlgorithm* instance);
	DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSectionCoord(AirVolumeAlgorithm* instance, int i, int** arr, int* size);
	DLLEXPORT void AirVolumeAlgorithm_GetMinimalCrossSectionNormal(AirVolumeAlgorithm* instance, int i, double** arr, int* size);

	DLLEXPORT MandibularCondylesAlgorithm* MandibularCondylesAlgorithm_CreateInstance();
	DLLEXPORT MandibularGeneralThresholdAlgorithm* MandibularGeneralThresholdAlgorithm_CreateInstance();
	DLLEXPORT MandibularTeethThresholdAlgorithm* MandibularTeethThresholdAlgorithm_CreateInstance();
	DLLEXPORT MandibularTeethDilateAlgorithm* MandibularTeethDilateAlgorithm_CreateInstance();
	DLLEXPORT MandibularSubtractAlgorithm* MandibularSubtractAlgorithm_CreateInstance();
	DLLEXPORT MandibularExtractionAlgorithm* MandibularExtractionAlgorithm_CreateInstance();
	DLLEXPORT UpperSkullExtractionAlgorithm* UpperSkullExtractionAlgorithm_CreateInstance();
	DLLEXPORT ZygomaticArchsExtractionAlgorithm* ZygomaticArchsExtractionAlgorithm_CreateInstance();

	DLLEXPORT void MandibularCondylesAlgorithm_Dispose(MandibularCondylesAlgorithm* instance) { delete instance; }
	DLLEXPORT void MandibularGeneralThresholdAlgorithm_Dispose(MandibularGeneralThresholdAlgorithm* instance) { delete instance; }
	DLLEXPORT void MandibularTeethThresholdAlgorithm_Dispose(MandibularTeethThresholdAlgorithm* instance) { delete instance; }
	DLLEXPORT void MandibularTeethDilateAlgorithm_Dispose(MandibularTeethDilateAlgorithm* instance) { delete instance; }
	DLLEXPORT void MandibularSubtractAlgorithm_Dispose(MandibularSubtractAlgorithm* instance) { delete instance; }
	DLLEXPORT void MandibularExtractionAlgorithm_Dispose(MandibularCondylesAlgorithm* instance) { delete instance; }
	DLLEXPORT void UpperSkullExtractionAlgorithm_Dispose(UpperSkullExtractionAlgorithm* instance) { delete instance; }
	DLLEXPORT void ZygomaticArchsExtractionAlgorithm_Dispose(ZygomaticArchsExtractionAlgorithm* instance) { delete instance; }

	DLLEXPORT int MandibularBaseAlgorithm_Run(MandibularBaseAlgorithm* instance, DicomSeries* dicomSeries);
	DLLEXPORT bool MandibularBaseAlgorithm_GetSuccess(MandibularBaseAlgorithm* instance);
	DLLEXPORT char* MandibularBaseAlgorithm_GetErrorMessage(MandibularBaseAlgorithm* instance);
	DLLEXPORT float MandibularBaseAlgorithm_GetProgress(MandibularBaseAlgorithm* instance);

	DLLEXPORT void MandibularCondylesAlgorithm_GetLeftCondylePosition(MandibularCondylesAlgorithm* instance, double** arr1, int* size1);
	DLLEXPORT void MandibularCondylesAlgorithm_GetRightCondylePosition(MandibularCondylesAlgorithm* instance, double** arr2, int* size2);
	DLLEXPORT void MandibularCondylesAlgorithm_GetCondylesPositionDifference(MandibularCondylesAlgorithm* instance, double** arr3, int* size3);
	DLLEXPORT double MandibularCondylesAlgorithm_GetCondylesDistance(MandibularCondylesAlgorithm* instance);
	DLLEXPORT double MandibularCondylesAlgorithm_GetCondylesDistancePx(MandibularCondylesAlgorithm* instance);
	DLLEXPORT void MandibularCondylesAlgorithm_GetNasionPoint(MandibularCondylesAlgorithm* instance, double** arr3, int* size3);

	DLLEXPORT void MandibularCondylesAlgorithm_GetLeft2NasionDifference(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT void MandibularCondylesAlgorithm_GetRight2NasionDifference(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT double MandibularCondylesAlgorithm_GetLeft2NasionDistancePx(MandibularCondylesAlgorithm* instance);
	DLLEXPORT double MandibularCondylesAlgorithm_GetLeft2NasionDistance(MandibularCondylesAlgorithm* instance);
	DLLEXPORT double MandibularCondylesAlgorithm_GetRight2NasionDistancePx(MandibularCondylesAlgorithm* instance);
	DLLEXPORT double MandibularCondylesAlgorithm_GetRight2NasionDistance(MandibularCondylesAlgorithm* instance);
	DLLEXPORT void MandibularCondylesAlgorithm_DumpIntermediates(MandibularCondylesAlgorithm* instance, const char* folder);

	DLLEXPORT void MandibularCondylesAlgorithm_GetLeftZygomaticPoint(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT void MandibularCondylesAlgorithm_GetRightZygomaticPoint(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT void MandibularCondylesAlgorithm_GetCenter(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT void MandibularCondylesAlgorithm_GetLeftToCenterDifferenceEx(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT void MandibularCondylesAlgorithm_GetRightToCenterDifferenceEx(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT double MandibularCondylesAlgorithm_GetLeftToCenterDistance(MandibularCondylesAlgorithm* instance);
	DLLEXPORT double MandibularCondylesAlgorithm_GetRightToCenterDistance(MandibularCondylesAlgorithm* instance);
	DLLEXPORT void MandibularCondylesAlgorithm_GetImageSpacing(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT void MandibularCondylesAlgorithm_GetImageOrigin(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT void MandibularCondylesAlgorithm_GetLeftCondyleReal(MandibularCondylesAlgorithm* instance, double** arr, int* size);
	DLLEXPORT void MandibularCondylesAlgorithm_GetRightCondyleReal(MandibularCondylesAlgorithm* instance, double** arr, int* size);

	DLLEXPORT double* DicomProcessing_GetTransform(double* srcPoints, double* srcCenter, double* trgPoints, double* trgCenter);
}