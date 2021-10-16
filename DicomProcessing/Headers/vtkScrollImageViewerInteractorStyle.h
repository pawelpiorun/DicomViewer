#pragma once

#include <vtk-9.0/vtkSmartPointer.h>
#include <vtk-9.0/vtkImageViewer2.h>
#include <vtk-9.0/vtkRenderWindowInteractor.h>
#include <vtk-9.0/vtkInteractorStyleImage.h>
#include <vtk-9.0/vtkObjectFactory.h>

class vtkScrollImageViewerInteractorStyle : public vtkInteractorStyleImage
{
public:
	static vtkScrollImageViewerInteractorStyle* New();
	vtkTypeMacro(vtkScrollImageViewerInteractorStyle, vtkInteractorStyleTrackballCamera);

protected:
	vtkImageViewer2* imageViewer;
	int current;
	int min;
	int max;

public:
	void SetImageViewer(vtkImageViewer2* imageViewer);
	void UpdateCurrentSlice(int index);

protected:
	void MoveSliceForward();
	void MoveSliceBackward();

	virtual void OnMouseWheelForward();
	virtual void OnMouseWheelBackward();
};