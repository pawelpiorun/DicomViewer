#ifndef VTKRENDER_HEADER
#define VTKRENDER_HEADER

#include <windows.h>
#include <vtk-9.0/vtkActor.h>
#include <vtk-9.0/vtkCamera.h>
#include <vtk-9.0/vtkCylinderSource.h>
#include <vtk-9.0/vtkNamedColors.h>
#include <vtk-9.0/vtkPolyDataMapper.h>
#include <vtk-9.0/vtkProperty.h>
#include <vtk-9.0/vtkRenderWindow.h>
#include <vtk-9.0/vtkRenderWindowInteractor.h>
#include <vtk-9.0/vtkRenderer.h>
#include <vtk-9.0/vtkSmartPointer.h>
#include <vtk-9.0/vtkDICOMImageReader.h>
#include <vtk-9.0/vtkImageData.h>
#include <vtk-9.0/vtkInteractorStyleTrackballCamera.h>
#include <vtk-9.0/vtkSmartVolumeMapper.h>
#include <vtk-9.0/vtkVolumeProperty.h>
#include <vtk-9.0/vtkPiecewiseFunction.h>
#include <vtk-9.0/vtkColorTransferFunction.h>
#include <vtk-9.0/vtkMultiVolume.h>
#include <vtk-9.0/vtkGPUVolumeRayCastMapper.h>
#include <vtk-9.0/vtkImageViewer2.h>
#include "vtk-9.0/vtkImageSlice.h"
#include "vtk-9.0/vtkImageResliceMapper.h"

#include "vtkScrollImageViewerInteractorStyle.h"

#include <vtk-9.0/vtkResliceImageViewer.h>

#include "DicomSeries.h"
#include "Presets.h"
#include "vtkRenderSync.h"

#include <vector>
#include <array>
#include "vtk-9.0/vtkTransform.h"

#include <vtk-9.0/vtkOrientationMarkerWidget.h>
#include <vtk-9.0/vtkAxesActor.h>
#include <vtk-9.0/vtkPropAssembly.h>

enum RenderType
{
	Volumetric = 0,
	FlatXY = 1,
	FlatXZ = 2,
	FlatYZ = 3
};

class VtkRender
{

public:
	VtkRender();
	~VtkRender();
	void SetRenderHost(HWND hwnd);
	bool LoadTestVolume();
	bool LoadDicomVolume(DicomSeries* dicomSeries);
	bool LoadAdditionalDicomVolume(DicomSeries* dicomSeries);
	void Render();
	//void StartRender();
	void UpdateData(vtkSmartPointer<vtkImageData> newData);
	void SetWindowSize(int width, int height);
	void SetVolumeProperty(Preset* preset, bool render = true);
	void SetPresets(Presets* presets);
	void SetRenderOption(RenderType option);
	void SetSliceIndex(int index);
	int GetSliceIndex();
	int GetMaxSliceIndex();
	vtkResliceImageViewer* GetReslicer() { return imageViewer.GetPointer(); };
	void ToggleResliceCursors(bool value);
	void SetRenderSync(VtkRenderSync* sync) { this->sync = sync; };
	void SetAdditionalVolumeTransform(double* transform);
	void ToggleShowSecondVolume(bool value);
	void SelectPoint(double x, double y, double z, bool onFirst = true);
	void ClearPointsSelection(bool onFirst = true);
private:
	std::vector<vtkSmartPointer<vtkActor>> selectedPoints;

	VtkRenderSync* sync = nullptr;
	RenderType renderOption = Volumetric;
	void PrepareVolume(DicomSeries* dicomSeries);
	void PrepareSlice(DicomSeries* dicomSeries);
	void ShowAxes();

	vtkSmartPointer<vtkImageData> imageData = nullptr;
	vtkSmartPointer<vtkImageData> imageData2 = nullptr;
	vtkSmartPointer<vtkRenderWindow> renderWindow;
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkVolume> volume;
	vtkSmartPointer<vtkVolume> volume2;
	bool isVolumePropertySet = false;
	vtkSmartPointer<vtkVolumeProperty> volumeProperty;
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper;
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper2;
	vtkSmartPointer<vtkMultiVolume> multiVolume;
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> multiVolumeMapper;
	HWND hwnd = nullptr;
	std::list<Preset*> presets;
	Preset* currentPreset;

	vtkSmartPointer<vtkResliceImageViewer> imageViewer;
	vtkSmartPointer<vtkInteractorStyleImage> imageViewerInteractor;
};

#endif