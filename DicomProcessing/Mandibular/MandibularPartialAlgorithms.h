#pragma once
#include "MandibularBaseAlgorithm.h"

class MandibularGeneralThresholdAlgorithm : public MandibularBaseAlgorithm
{
public:
	MandibularGeneralThresholdAlgorithm() { };
	~MandibularGeneralThresholdAlgorithm() { }
	ImageSource<ImageType>::Pointer GetLastPipelineObject()
	{
		auto processObjects = this->GetProcessObjects();
		return processObjects[3];
	}
};

class MandibularTeethThresholdAlgorithm : public MandibularBaseAlgorithm
{
public:
	MandibularTeethThresholdAlgorithm() { };
	~MandibularTeethThresholdAlgorithm() { }
	ImageSource<ImageType>::Pointer GetLastPipelineObject()
	{
		auto processObjects = this->GetProcessObjects();
		return processObjects[0];
	}
};

class MandibularTeethDilateAlgorithm : public MandibularBaseAlgorithm
{
public:
	MandibularTeethDilateAlgorithm() { };
	~MandibularTeethDilateAlgorithm() { }
	ImageSource<ImageType>::Pointer GetLastPipelineObject()
	{
		auto processObjects = this->GetProcessObjects();
		return processObjects[2];
	}
};

class MandibularSubtractAlgorithm : public MandibularBaseAlgorithm
{
public:
	MandibularSubtractAlgorithm() { };
	~MandibularSubtractAlgorithm() { }
	ImageSource<ImageType>::Pointer GetLastPipelineObject()
	{
		auto processObjects = this->GetProcessObjects();
		return processObjects[4];
	}
};

class MandibularExtractionAlgorithm : public MandibularBaseAlgorithm
{
public:
	MandibularExtractionAlgorithm() { };
	~MandibularExtractionAlgorithm() { }
	ImageSource<ImageType>::Pointer GetLastPipelineObject()
	{
		auto processObjects = this->GetProcessObjects();
		return processObjects[5];
	}
};

class UpperSkullExtractionAlgorithm : public MandibularBaseAlgorithm
{
public:
	UpperSkullExtractionAlgorithm() { };
	~UpperSkullExtractionAlgorithm() { }
	ImageSource<ImageType>::Pointer GetLastPipelineObject()
	{
		auto processObjects = this->GetProcessObjects();
		return processObjects[6];
	}
};

class ZygomaticArchsExtractionAlgorithm : public MandibularBaseAlgorithm
{
public:
	ZygomaticArchsExtractionAlgorithm() { MandibularBaseAlgorithm::endWithZygomaticArchs = true; };
	~ZygomaticArchsExtractionAlgorithm() { }
	ImageSource<ImageType>::Pointer GetLastPipelineObject()
	{
		auto processObjects = this->GetProcessObjects();
		return processObjects[6];
	}
};