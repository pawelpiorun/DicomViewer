#ifndef VTKRENDERSYNC_HEADER
#define VTKREDNERSYNC_HEADER

#include "vtk-9.0/vtkRenderWindowInteractor.h"
#include "vtk-9.0/vtkResliceImageViewer.h"
#include "vtk-9.0/vtkResliceCursor.h"
#include "vtk-9.0/vtkResliceCursorWidget.h"
#include "vtk-9.0/vtkResliceCursorActor.h"
#include "vtk-9.0/vtkResliceCursorLineRepresentation.h"
#include "vtk-9.0/vtkResliceCursorPolyDataAlgorithm.h"
#include "vtk-9.0/vtkRenderWindow.h"
#include "vtk-9.0/vtkRenderer.h"
#include "vtk-9.0/vtkCamera.h"
#include "vtk-9.0/vtkRendererCollection.h"

class vtkResliceCursorCallback : public vtkCommand
{
public:
	static vtkResliceCursorCallback* New()
	{
		return new vtkResliceCursorCallback;
	}

	void Execute(vtkObject* caller, unsigned long ev,
		void* callData)
	{
		if (ev == vtkResliceCursorWidget::WindowLevelEvent ||
			ev == vtkCommand::WindowLevelEvent ||
			ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
		{
			// Render everything
			for (int i = 0; i < 3; i++)
			{
				this->RCW[i]->Render();
			}
			return;
		}
		else if (ev == vtkCommand::MouseWheelBackwardEvent
			|| ev == vtkCommand::MouseWheelForwardEvent)
		{
			vtkRenderWindowInteractor* rwi = dynamic_cast<
				vtkRenderWindowInteractor*>(caller);

			auto ren = rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
			if (ren)
			{
				auto scale = ren->GetActiveCamera()->GetParallelScale();
				for (int i = 0; i < 3; i++)
				{
					RCW[i]->GetCurrentRenderer()->GetActiveCamera()->SetParallelScale(scale);
				}
			}
		}

		for (int i = 0; i < 3; i++)
		{
			this->RCW[i]->Render();
		}
	}

	vtkResliceCursorCallback() {}
	vtkResliceCursorWidget* RCW[3];
};

class VtkRenderSync
{
public:
	VtkRenderSync()
	{
		Cleanup();
	};
	~VtkRenderSync()
	{
		Cleanup();
	}

	void RegisterReslicer(vtkResliceImageViewer* rw);
	void SyncReslicers();
	void SetEnabled(bool isEnabled);
	void SetResliceCenter(double x, double y, double z);
	void SetResliceNormal(double x, double y, double z);
	void ResetReslicers();
	void Cleanup();

private:
	vtkResliceImageViewer* riw[3];
	vtkSmartPointer<vtkResliceCursorCallback> cbk = nullptr;
	double* initialOrigin = nullptr;
};

#endif