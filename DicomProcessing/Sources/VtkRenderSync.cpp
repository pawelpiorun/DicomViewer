#include "vtkRenderSync.h"
#include "vtk-9.0/vtkPlane.h"
#include "vtk-9.0/vtkImageData.h"
#include "vtk-9.0/vtkLookupTable.h"

void VtkRenderSync::Cleanup()
{
	riw[0] = nullptr;
	riw[1] = nullptr;
	riw[2] = nullptr;
	if (cbk != nullptr)
	{
		for (int i = 0; i < 3; i++)
			cbk->RCW[i] = nullptr;
		cbk = nullptr;
	}
}

void VtkRenderSync::RegisterReslicer(vtkResliceImageViewer* rw)
{
	if (riw[0] == nullptr)
		riw[0] = rw;
	else if (riw[1] == nullptr)
		riw[1] = rw;
	else if (riw[2] == nullptr)
	{
		riw[2] = rw;
		SyncReslicers();
	}
}

void VtkRenderSync::SyncReslicers()
{
	cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();

	for (int i = 0; i < 3; i++)
	{
		vtkResliceCursorLineRepresentation* rep =
			vtkResliceCursorLineRepresentation::SafeDownCast(
				riw[i]->GetResliceCursorWidget()->GetRepresentation());
		riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());
		rep->GetResliceCursorActor()->
			GetCursorAlgorithm()->SetReslicePlaneNormal(i);
		cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);
		riw[i]->GetInteractor()->AddObserver(vtkCommand::MouseWheelBackwardEvent, cbk);
		riw[i]->GetInteractor()->AddObserver(vtkCommand::MouseWheelForwardEvent, cbk);
		riw[i]->GetResliceCursorWidget()->SetEnabled(1);

		auto lut = vtkLookupTable::SafeDownCast(rep->GetLookupTable());
		lut->SetUseAboveRangeColor(true);
		lut->SetAboveRangeColor(255, 0, 0, 255);
	}

	//riw[0]->GetResliceCursor()->SetCenter(250, 250, 100);
	/*riw[0]->GetResliceCursor()->SetThickMode(1);*/
	riw[0]->GetResliceCursor()->SetThickness(1, 1, 1);
	riw[0]->GetResliceCursor()->SetHole(0);		
	std::cout << "Reslicers synchronization done." << std::endl;

	for (int i = 0; i < 3; i++)
		riw[i]->Render();

	for (int i = 0; i < 3; i++)
	{
		vtkResliceCursorLineRepresentation* rep =
			vtkResliceCursorLineRepresentation::SafeDownCast(
				riw[i]->GetResliceCursorWidget()->GetRepresentation());

		rep->GetRenderer()->GetActiveCamera()->SetParallelProjection(true);
		rep->GetRenderer()->GetActiveCamera()->SetParallelScale(83);
	}
}

void VtkRenderSync::SetEnabled(bool isEnabled)
{
	if (isEnabled)
	{
		SyncReslicers();
		return;
	}

	for (int i = 0; i < 3; i++)
	{
		cbk->RCW[i]->SetEnabled(0);
		riw[i]->GetResliceCursorWidget()->RemoveAllObservers();
	}

}


void VtkRenderSync::SetResliceCenter(double x, double y, double z)
{
	//std::cout << "Settings reslice center: " << x << ", " << y << ", " << z << std::endl;

	vtkResliceCursorRepresentation* rep1 =
		vtkResliceCursorRepresentation::SafeDownCast(
			riw[0]->GetResliceCursorWidget()->GetRepresentation());

	//auto origin = rep1->GetResliceCursor()->GetPlane(0)->GetOrigin();
	//std::cout << "Origin: " << origin[0] << origin[1] << origin[2] << std::endl;

	// WE HAVE TO CONVERT TO PHYSICAL COORDINATES SYSTEM
	auto spacing = riw[0]->GetInput()->GetSpacing();
	auto imageOrigin = riw[0]->GetInput()->GetOrigin();

	auto newX = imageOrigin[0] + x * spacing[0];
	auto newY = imageOrigin[1] + y * spacing[1];
	auto newZ = imageOrigin[2] + z * spacing[2];

	rep1->GetResliceCursor()->SetCenter(newX, newY, newZ);

}

void VtkRenderSync::SetResliceNormal(double x, double y, double z)
{
	// TODO
	//std::cout << "Settings reslice normal: " << x << ", " << y << ", " << z << std::endl;
	vtkResliceCursorLineRepresentation* rep1 =
		vtkResliceCursorLineRepresentation::SafeDownCast(
			riw[0]->GetResliceCursorWidget()->GetRepresentation());

	// red = X == 0
	// green = Y == 1
	// blue = Z == 2

	//auto normal = rep1->GetResliceCursor()->GetPlane(2)->GetNormal();
	//std::cout << "Normal: " << normal[0] << normal[1] << normal[2] << std::endl;

	//rep1->GetResliceCursor()->GetPlane(0)->SetNormal(1, std::abs(y), 0);
	//rep1->GetResliceCursor()->GetPlane(1)->SetNormal(std::abs(x), 1, 0);

	rep1->GetResliceCursor()->GetPlane(2)->SetNormal(x, y, 1);
	for (int i = 0; i < 3; i++)
		riw[i]->Render();
}

void VtkRenderSync::ResetReslicers()
{
	riw[0]->GetResliceCursorWidget()->ResetResliceCursor();
	riw[1]->GetResliceCursorWidget()->ResetResliceCursor();
	riw[2]->GetResliceCursorWidget()->ResetResliceCursor();

	SetEnabled(false);
	SetEnabled(true);
	
	for (int i = 0; i < 3; i++)
		riw[i]->Render();

	std::cout << "Reslicers reset." << std::endl;
}