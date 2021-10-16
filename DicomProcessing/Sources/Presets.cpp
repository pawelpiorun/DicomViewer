#include "Presets.h"

Presets::Presets()
{

}

Presets::Presets(const Presets &)
{

}

Presets::~Presets()
{
	this->presets.clear();
}

Preset* Presets::CreatePreset()
{
	auto preset = new Preset();
	this->presets.push_back(preset);
	return preset;
}

std::list<Preset*> Presets::GetPresets()
{
	return this->presets;
}

void Presets::LoadPresets()
{
	auto preset = CreatePreset();
	preset->SetProperties(1, 0, 1, 0.2, 1);
	preset->AddGradientPoint(0, 1);
	preset->AddGradientPoint(255, 1);
	preset->AddScalarPoint(0, 0);
	preset->AddScalarPoint(20, 0);
	preset->AddScalarPoint(40, 0.15);
	preset->AddScalarPoint(120, 0.3);
	preset->AddScalarPoint(220, 0.375);
	preset->AddScalarPoint(1024, 0.5);
	preset->AddScalarPoint(SHRT_MAX, 0.7);
	preset->AddColorTransfer(0, 0, 0, 0);
	preset->AddColorTransfer(20, 0.168627, 0, 0);
	preset->AddColorTransfer(40, 0.403922, 0.145098, 0.0784314);
	preset->AddColorTransfer(120, 0.780392, 0.607843, 0.380392);
	preset->AddColorTransfer(220, 0.847059, 0.835294, 0.788235);
	preset->AddColorTransfer(1024, 1, 1, 1);
	preset->AddColorTransfer(SHRT_MAX, 1, 0, 0);
}

Preset::Preset()
{

}
Preset::~Preset()
{
	gradientOpacity.clear();
	scalarOpacity.clear();
	colorTransfer.clear();
}
void Preset::SetName(char* name)
{
	this->name = name;
}
void Preset::SetProperties(double specularPower, double specular,
	double shade, double ambient, double diffuse)
{
	this->specularPower = specularPower;
	this->specular = specular;
	this->shade = shade;
	this->ambient = ambient;
	this->diffuse = diffuse;
}
void Preset::AddGradientPoint(double x, double y)
{
	auto p = Point();
	p.x = x;
	p.y = y;
	this->gradientOpacity.push_back(p);
}
void Preset::AddScalarPoint(double x, double y)
{
	auto p = Point();
	p.x = x;
	p.y = y;
	this->scalarOpacity.push_back(p);
}
void Preset::AddColorTransfer(double x, double r, double g, double b)
{
	auto rgbct = RGBColorTransfer();
	rgbct.x = x;
	rgbct.r = r;
	rgbct.g = g;
	rgbct.b = b;
	this->colorTransfer.push_back(rgbct);
}

char* Preset::GetName()
{
	return this->name;
}
double Preset::GetSpecularPower()
{
	return this->specularPower;
}
double Preset::GetSpecular()
{
	return this->specular;
}
double Preset::GetShade()
{
	return this->shade;
}
double Preset::GetAmbient()
{
	return this->ambient;
}
double Preset::GetDiffuse()
{
	return this->diffuse;
}
std::list<Point> Preset::GetGradientOpacity()
{
	return this->gradientOpacity;
}
std::list<Point> Preset::GetScalarOpacity()
{
	return this->scalarOpacity;
}
std::list<RGBColorTransfer> Preset::GetColorTransfer()
{
	return this->colorTransfer;
}