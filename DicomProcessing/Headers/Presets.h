#pragma once
#include <list>

typedef struct
{
	double x;
	double y;
} Point;

typedef struct
{
	double x;
	double r;
	double g;
	double b;
} RGBColorTransfer;

class Preset
{
public:
	Preset();
	~Preset();

	void SetName(char* name);
	void SetProperties(double specularPower, double specular,
		double shade, double ambient, double diffues);
	void AddGradientPoint(double x, double y);
	void AddScalarPoint(double x, double y);
	void AddColorTransfer(double x, double r, double g, double b);

	char* GetName();
	double GetSpecularPower();
	double GetSpecular();
	double GetShade();
	double GetAmbient();
	double GetDiffuse();
	std::list<Point> GetGradientOpacity();
	std::list<Point> GetScalarOpacity();
	std::list<RGBColorTransfer> GetColorTransfer();

private:
	char* name;
	double specularPower;
	double specular;
	double shade;
	double ambient;
	double diffuse;
	std::list<Point> gradientOpacity;
	std::list<Point> scalarOpacity;
	std::list<RGBColorTransfer> colorTransfer;
};

class Presets
{
private:
	Presets();
	Presets(const Presets &);
	std::list<Preset*> presets = std::list<Preset*>();

public:
	~Presets();
	static Presets & GetInstance()
	{
		static Presets singleton;
		return singleton;
	}

	Preset* CreatePreset();
	std::list<Preset*> GetPresets();

	// only for C++ .exe for now
	void LoadPresets();
	//void LoadPresets(const wchar_t* fileName);
};