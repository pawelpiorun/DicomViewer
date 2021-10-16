#pragma once
#include "MandibularBaseAlgorithm.h"

class MandibularCondylesAlgorithm : public MandibularBaseAlgorithm
{
public:
	MandibularCondylesAlgorithm() { }
	~MandibularCondylesAlgorithm() { }
	ImageSource<ImageType>::Pointer GetLastPipelineObject()
	{
		auto processObjects = this->GetProcessObjects();
		return processObjects.back();
	}
	void DumpIntermediates(const char* outputFolder);
};