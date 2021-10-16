#include "vtkScrollImageViewerInteractorStyle.h"

vtkStandardNewMacro(vtkScrollImageViewerInteractorStyle);

void vtkScrollImageViewerInteractorStyle::SetImageViewer(vtkImageViewer2* imageViewer)
{
	this->imageViewer = imageViewer;
	min = imageViewer->GetSliceMin();
	max = imageViewer->GetSliceMax();
	current = min;
#if DEBUG
	cout << "Slicer: Min = " << min << ", Max = " << max << std::endl;
#endif
}

void vtkScrollImageViewerInteractorStyle::MoveSliceForward()
{
	if (current < max) {
		current += 1;
#if DEBUG
		cout << "MoveSliceForward::Slice = " << current << std::endl;
#endif
		imageViewer->SetSlice(current);
		imageViewer->Render();
	}
}

void vtkScrollImageViewerInteractorStyle::MoveSliceBackward()
{
	if (current > min) {
		current -= 1;
#if DEBUG
		cout << "MoveSliceBackward::Slice = " << current << std::endl;
#endif
		imageViewer->SetSlice(current);
		imageViewer->Render();
	}
}

void vtkScrollImageViewerInteractorStyle::OnMouseWheelForward()
{
	if (this->Interactor->GetShiftKey())
	{
		MoveSliceForward();
	}
	else
	{
		vtkInteractorStyleImage::OnMouseWheelForward();
	}
}


void vtkScrollImageViewerInteractorStyle::OnMouseWheelBackward()
{
	if (this->Interactor->GetShiftKey())
	{
		if (current > min) {
			MoveSliceBackward();
		}
	}
	else
	{
		vtkInteractorStyleImage::OnMouseWheelBackward();
	}
}

void vtkScrollImageViewerInteractorStyle::UpdateCurrentSlice(int index)
{
	current = index;
}