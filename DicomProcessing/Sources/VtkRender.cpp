#include "vtkRender.h"
#include "vtk-9.0/vtkImageMapToWindowLevelColors.h"
#include "vtk-9.0/vtkLookupTable.h"
#include "vtk-9.0/vtkMatrix4x4.h"
#include "vtk-9.0/vtkSphereSource.h"
#include "vtk-9.0/vtkCaptionActor2D.h"
#include "vtk-9.0/vtkTextProperty.h"
#include "vtk-9.0/vtkTextActor.h"

VtkRender::VtkRender()
{
	imageData = vtkSmartPointer<vtkImageData>::New();
	imageData2 = vtkSmartPointer<vtkImageData>::New();
	renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	volume = vtkSmartPointer<vtkVolume>::New();
	volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volume2 = vtkSmartPointer<vtkVolume>::New();
	volumeMapper2 = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	multiVolume = vtkSmartPointer<vtkMultiVolume>::New();
	multiVolumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();

	imageViewer = vtkSmartPointer<vtkResliceImageViewer>::New();
}

void VtkRender::SetRenderHost(HWND handle)
{
	this->hwnd = handle;
}

void VtkRender::SetWindowSize(int width, int height)
{
	renderWindow->SetSize(width, height);
}

void VtkRender::SetVolumeProperty(Preset* preset, bool render)
{
	if (currentPreset == preset) return;

	vtkSmartPointer<vtkPiecewiseFunction> gradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	vtkSmartPointer<vtkPiecewiseFunction> scalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
	this->currentPreset = preset;

	if ((int)(preset->GetShade()) == 1)
		volumeProperty->ShadeOn();
	else
		volumeProperty->ShadeOff();

	volumeProperty->SetInterpolationTypeToLinear();

	volumeProperty->SetAmbient(preset->GetAmbient());
	volumeProperty->SetDiffuse(preset->GetDiffuse());
	volumeProperty->SetSpecular(preset->GetSpecular());
	volumeProperty->SetSpecularPower(preset->GetSpecularPower());

	auto gradientList = preset->GetGradientOpacity();
	auto gradient = gradientList.begin();
	for (; gradient != gradientList.end(); gradient++)
		gradientOpacity->AddPoint(gradient->x, gradient->y);

	auto scalarList = preset->GetScalarOpacity();
	auto scalar = scalarList.begin();
	for (; scalar != scalarList.end(); scalar++)
		scalarOpacity->AddPoint(scalar->x, scalar->y);

	auto colorList = preset->GetColorTransfer();
	auto newColor = colorList.begin();
	for (; newColor != colorList.end(); newColor++)
		color->AddRGBPoint(newColor->x, newColor->r, newColor->g, newColor->b);

	volumeProperty->SetGradientOpacity(gradientOpacity);
	volumeProperty->SetScalarOpacity(scalarOpacity);
	volumeProperty->SetColor(color);
	//renderWindow->SetStereoTypeToAnaglyph();

	isVolumePropertySet = true;
	if (render)
		renderWindow->Render();
}

void VtkRender::SetPresets(Presets* presets)
{
	this->presets = presets->GetPresets();
}

bool VtkRender::LoadTestVolume()
{
	vtkSmartPointer<vtkNamedColors> colors =
		vtkSmartPointer<vtkNamedColors>::New();

	// Set the background color.
	std::array<unsigned char, 4> bkg{ {50, 50, 50, 255} };
	colors->SetColor("BkgColor", bkg.data());

	// This creates a polygonal cylinder model with eight circumferential facets
	// (i.e, in practice an octagonal prism).
	vtkSmartPointer<vtkCylinderSource> cylinder =
		vtkSmartPointer<vtkCylinderSource>::New();
	cylinder->SetResolution(8);

	// The mapper is responsible for pushing the geometry into the graphics
	// library. It may also do color mapping, if scalars or other attributes are
	// defined.
	vtkSmartPointer<vtkPolyDataMapper> cylinderMapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

	// The actor is a grouping mechanism: besides the geometry (mapper), it
	// also has a property, transformation matrix, and/or texture map.
	// Here we set its color and rotate it around the X and Y axes.
	vtkSmartPointer<vtkActor> cylinderActor = vtkSmartPointer<vtkActor>::New();
	cylinderActor->SetMapper(cylinderMapper);
	cylinderActor->GetProperty()->SetColor(
		colors->GetColor4d("Tomato").GetData());
	cylinderActor->RotateX(30.0);
	cylinderActor->RotateY(-45.0);

	// The renderer generates the image
	// which is then displayed on the render window.
	// It can be thought of as a scene to which the actor is added
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(cylinderActor);
	renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
	// Zoom in a little by accessing the camera and invoking its "Zoom" method.
	renderer->ResetCamera();
	renderer->GetActiveCamera()->Zoom(1.5);

	// The render window is the actual GUI window
	// that appears on the computer screen
	renderWindow->AddRenderer(renderer);
	renderWindow->SetWindowName("Cylinder");

	// The render window interactor captures mouse events
	// and will perform appropriate camera or actor manipulation
	// depending on the nature of the events.
	renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	if (this->hwnd != nullptr)
	{
		renderWindow->SetParentId(this->hwnd);
		renderWindowInteractor->Initialize();
	}

	// This starts the event loop and as a side effect causes an initial render.
	renderWindow->Render();

	if (this->hwnd == nullptr)
		renderWindowInteractor->Start();

	HWND window = (HWND)renderWindow->GetGenericWindowId();
	LONG style = GetWindowLong(window, GWL_STYLE) & ~(WS_BORDER | WS_DLGFRAME | WS_THICKFRAME);
	SetWindowLong(window, -16L, style);

	return true;
}

void VtkRender::UpdateData(vtkSmartPointer<vtkImageData> newData)
{
	imageData->ShallowCopy(newData);
	//imageData->Modified();
}

bool VtkRender::LoadDicomVolume(DicomSeries* dicomSeries)
{
	imageData->ShallowCopy(dicomSeries->GetVtkImageData());

	if (renderOption == Volumetric)
		PrepareVolume(dicomSeries);
	else
		PrepareSlice(dicomSeries);

	return true;
}


void VtkRender::SetAdditionalVolumeTransform(double* transform)
{
	if (imageData2 == nullptr) return;

	auto matrix = vtkMatrix4x4::New();
	matrix->SetElement(0, 0, transform[0]);
	matrix->SetElement(0, 1, transform[1]);
	matrix->SetElement(0, 2, transform[2]);
	matrix->SetElement(0, 3, transform[3]);

	matrix->SetElement(1, 0, transform[4]);
	matrix->SetElement(1, 1, transform[5]);
	matrix->SetElement(1, 2, transform[6]);
	matrix->SetElement(1, 3, transform[7]);

	matrix->SetElement(2, 0, transform[8]);
	matrix->SetElement(2, 1, transform[9]);
	matrix->SetElement(2, 2, transform[10]);
	matrix->SetElement(2, 3, transform[11]);

	matrix->SetElement(3, 0, transform[12]);
	matrix->SetElement(3, 1, transform[13]);
	matrix->SetElement(3, 2, transform[14]);
	matrix->SetElement(3, 3, transform[15]);

	volume2->SetUserMatrix(matrix);
	this->Render();
}

void VtkRender::SelectPoint(double x, double y, double z, bool onFirst)
{
	vtkSmartPointer<vtkNamedColors> colors =
		vtkSmartPointer<vtkNamedColors>::New();

	// Create a sphere
	vtkSmartPointer<vtkSphereSource> sphereSource =
		vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->SetCenter(x, y, z);
	sphereSource->SetRadius(2.0);
	sphereSource->SetPhiResolution(100);
	sphereSource->SetThetaResolution(100);

	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(sphereSource->GetOutputPort());

	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	const char* color = onFirst ? "DarkBlue" : "Red";
	std::cout << color << " " << x << "," << y << "," << z << std::endl;

	actor->GetProperty()->SetColor(colors->GetColor3d(color).GetData());
	
	renderer->AddActor(actor);
	selectedPoints.push_back(actor);

	Render();
}

void VtkRender::ClearPointsSelection(bool onFirst)
{
	vtkSmartPointer<vtkNamedColors> colors =
		vtkSmartPointer<vtkNamedColors>::New();
	auto red = colors->GetColor3d("Red").GetData();
	auto blue = colors->GetColor3d("DarkBlue").GetData();

	for (int i = 0; i < selectedPoints.size(); i++)
	{
		auto prop = selectedPoints[i];
		auto color = prop->GetProperty()->GetColor();
		bool remove = false;
		if (onFirst)
		{
			if (color[0] == blue[0])
				remove = true;
		}
		else
		{
			if (color[0] == red[0])
				remove = true;
		}
		if (remove)
		{
			auto color = onFirst ? " blue" : " red";
			std::cout << "Removing" << color << std::endl;
			renderer->RemoveActor(selectedPoints[i]);
		}
	}
}

void VtkRender::ToggleShowSecondVolume(bool value)
{
	if (imageData2 == nullptr) return;
	std::cout << "vis: " << value << std::endl;
	if (value)
		volume2->SetVisibility(value);
	else
		volume2->SetVisibility(value);
}

bool VtkRender::LoadAdditionalDicomVolume(DicomSeries* dicomSeries)
{
	auto itkData = dicomSeries->GetItkImageData();
	auto size = itkData->GetLargestPossibleRegion().GetSize();
	for (int x = 0; x < size[0]; x++)
		for (int y = 0; y < size[1]; y++)
			for (int z = 0; z < size[2]; z++)
			{
				if (itkData->GetPixel(itk::Index<3U> { x, y, z}) >= 1000)
					itkData->SetPixel(itk::Index<3U> {x, y, z}, 5000);
			}
	
	imageData2->ShallowCopy(dicomSeries->GetVtkImageData());

	if (renderOption == Volumetric)
	{
		//renderer->SetBackground(0.3, 0.3, 0.3);

		//if (!renderWindow->HasRenderer(renderer))
		//	renderWindow->AddRenderer(renderer);

		//vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
		//renderWindowInteractor->SetInteractorStyle(interactorStyle);
		//renderWindowInteractor->SetRenderWindow(renderWindow);

		volumeMapper2->SetBlendModeToComposite();
		volumeMapper2->SetRequestedRenderModeToGPU();
		volumeMapper2->SetInputData(imageData2);
		
		if (!isVolumePropertySet)
			SetVolumeProperty(*presets.begin(), false);

		volume2->SetMapper(volumeMapper2);
		volume2->SetProperty(volumeProperty);

		multiVolumeMapper->SetBlendModeToComposite();
		multiVolumeMapper->SetInputDataObject(0, imageData);
		multiVolumeMapper->SetInputDataObject(1, imageData2);
		//multiVolumeMapper->UseJitteringOn();

		multiVolume->SetMapper(multiVolumeMapper);
		multiVolume->SetProperty(volumeProperty);
		multiVolume->SetVolume(volume, 0);
		multiVolume->SetVolume(volume2, 1);

		renderer->RemoveAllViewProps();
		if (renderer->VisibleVolumeCount() <= 1)
			renderer->AddVolume(multiVolume);

		//ShowAxes();
		renderer->ResetCamera();
	}

	return true;
}

void VtkRender::ShowAxes()
{
	vtkSmartPointer<vtkAxesActor> axes =
		vtkSmartPointer<vtkAxesActor>::New();	
	axes->SetTotalLength(30, 30, 30);
	axes->SetShaftTypeToLine();
	axes->SetCylinderRadius(0.005);
	
	vtkSmartPointer<vtkOrientationMarkerWidget> widget =
		vtkSmartPointer<vtkOrientationMarkerWidget>::New();
	widget->SetOutlineColor(0.9300, 0.5700, 0.1300);
	widget->SetOrientationMarker(axes);
	widget->SetInteractor(renderWindowInteractor);
	widget->SetViewport(0.80, 0.0, 1.0, 0.25);
	widget->SetEnabled(1);
	widget->InteractiveOff();
}

void VtkRender::PrepareVolume(DicomSeries* dicomSeries)
{
	renderer->SetBackground(0.3, 0.3, 0.3);

	if (!renderWindow->HasRenderer(renderer))
		renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	renderWindowInteractor->SetInteractorStyle(interactorStyle);
	renderWindowInteractor->SetRenderWindow(renderWindow);
	
	volumeMapper->SetBlendModeToComposite();
	volumeMapper->SetRequestedRenderModeToGPU();
	volumeMapper->SetInputData(imageData);

	if (!isVolumePropertySet)
		SetVolumeProperty(*presets.begin(), false);

	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	if (renderer->VisibleVolumeCount() == 0)
		renderer->AddVolume(volume);
	renderer->ResetCamera();
}

void VtkRender::Render()
{
	if (this->hwnd != nullptr)
	{
		renderWindow->SetParentId(this->hwnd);
		if (renderOption == Volumetric)
			renderWindowInteractor->Initialize();
	}

	renderWindow->GlobalWarningDisplayOff();
	renderWindow->Render();
	if (this->hwnd == nullptr)
		renderWindowInteractor->Start();
}

void VtkRender::PrepareSlice(DicomSeries* dicomSeries)
{
	imageViewer->OffScreenRenderingOn();
	imageViewer->SetInputData(imageData);
	imageViewer->SetResliceModeToAxisAligned();

	// colors stuff
	// http://vtk.1045678.n5.nabble.com/vtkImageViewer2-td3409087.html

	// set grayscale and selection special color
	imageViewer->GetWindowLevel()->SetOutputFormatToRGB();
	auto lut = vtkLookupTable::New();
	lut->SetAboveRangeColor(1, 0, 0, 1);
	lut->SetUseAboveRangeColor(true);
	lut->SetHueRange(0, 0);
	lut->SetSaturationRange(0, 0);
	lut->SetRange(SHRT_MIN, SHRT_MAX - 1); // leave the max value for selection
	lut->Build();
	imageViewer->GetWindowLevel()->SetLookupTable(lut);

	// look for image min and max
	using MinMaxCalcType = itk::MinimumMaximumImageCalculator<DicomSeriesType>;
	auto minMaxCalc = MinMaxCalcType::New();
	minMaxCalc->SetImage(dicomSeries->GetItkImageData());
	minMaxCalc->ComputeMaximum();
	minMaxCalc->ComputeMinimum();

	// normalize
	auto window = minMaxCalc->GetMaximum() - minMaxCalc->GetMinimum();
	imageViewer->SetColorWindow(window);

	if (this->hwnd != nullptr)
		renderWindow->SetParentId(this->hwnd);
	imageViewer->SetRenderWindow(renderWindow);

	int extent[6];
	imageData->GetExtent(extent);

	// TODO: Make it legit
	if (renderOption == FlatXY)
	{
		imageViewer->SetSliceOrientationToXY();
		imageViewer->SetSlice(0);
	}
	else if (renderOption == FlatXZ)
	{
		imageViewer->SetSliceOrientationToXZ();
		imageViewer->SetSlice(extent[3]);
	}
	else if (renderOption == FlatYZ)
	{
		imageViewer->SetSliceOrientationToYZ();
		imageViewer->SetSlice(0);
	}
	
	//if (renderOption == FlatXY)
	//	imageViewer->SetSlice(extent[5] / 2);
	//else if (renderOption == FlatXZ)
	//	imageViewer->SetSlice(extent[3] / 2);
	//else if (renderOption == FlatYZ)
	//	imageViewer->SetSlice(extent[1] / 2);

	//imageViewer->GetRenderer()->ResetCamera();
	//imageViewer->GetRenderer()->GetActiveCamera()->ParallelProjectionOn();
	//imageViewer->GetRenderer()->GetActiveCamera()->SetParallelScale(80);
	imageViewer->SetupInteractor(renderWindowInteractor);

	imageViewerInteractor = vtkScrollImageViewerInteractorStyle::New();
	auto scrollInteractor = dynamic_cast<vtkScrollImageViewerInteractorStyle*>(imageViewerInteractor.Get());
	if (scrollInteractor != nullptr)
	{
		scrollInteractor->SetImageViewer(imageViewer);
		scrollInteractor->UpdateCurrentSlice(imageViewer->GetSlice());
	}

	//renderWindowInteractor->SetInteractorStyle(imageViewerInteractor);
	renderWindowInteractor->SetRenderWindow(renderWindow);

}

void VtkRender::SetSliceIndex(int index)
{
	auto scrollInteractor = dynamic_cast<vtkScrollImageViewerInteractorStyle*>(imageViewerInteractor.Get());
	if (scrollInteractor != nullptr)
		scrollInteractor->UpdateCurrentSlice(index);
	imageViewer->SetSlice(index);
	imageViewer->Render();
}

void VtkRender::ToggleResliceCursors(bool value)
{
	// TODO
	if (sync == nullptr) return;

	if (value)
	{
		renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
		imageViewer->SetupInteractor(renderWindowInteractor);
		renderWindow->SetInteractor(renderWindowInteractor);

		sync->SetEnabled(value);
		if (renderOption == FlatXY)
			imageViewer->SetSlice(0);
		else if (renderOption == FlatXZ)
			imageViewer->SetSlice(0);
		else if (renderOption == FlatYZ)
			imageViewer->SetSlice(0);
	}
	else
	{
		renderWindowInteractor->SetInteractorStyle(imageViewerInteractor);
		sync->SetEnabled(value);

		int extent[6];
		imageData->GetExtent(extent);
		if (renderOption == FlatXY)
			imageViewer->SetSlice(extent[5] / 2);
		else if (renderOption == FlatXZ)
			imageViewer->SetSlice(extent[3] / 2);
		else if (renderOption == FlatYZ)
			imageViewer->SetSlice(extent[1] / 2);
	}
}

int VtkRender::GetSliceIndex() { return imageViewer->GetSlice(); }
int VtkRender::GetMaxSliceIndex() { return imageViewer->GetSliceMax(); }

void VtkRender::SetRenderOption(RenderType option)
{
	this->renderOption = option;
}

VtkRender::~VtkRender()
{
	this->hwnd = nullptr;
	this->renderWindow = nullptr;
}