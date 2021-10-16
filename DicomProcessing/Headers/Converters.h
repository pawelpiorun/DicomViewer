#pragma once
#include "ITK-5.1/itkSmartPointer.h"
#include "ITK-5.1/itkCastImageFilter.h"
#include "ITK-5.1/itkRescaleIntensityImageFilter.h"

#include "vtk-9.0/vtkSmartPointer.h"
#include "ITK-5.1/itkImageToVTKImageFilter.h"
#include "ITK-5.1/itkVTKImageToImageFilter.h"

namespace Converters
{
	template <typename TIn, typename TOut>
	inline itk::SmartPointer<TOut> Convert(itk::SmartPointer<TIn> image)
	{
		typedef itk::CastImageFilter<TIn, TOut> CastOutputType;
		CastOutputType::Pointer castOutput = CastOutputType::New();
		castOutput->SetInput(image);
		castOutput->Update();
		return castOutput->GetOutput();
	}

	template <typename TIn, typename TOut>
	inline itk::SmartPointer<TOut> Rescale(itk::SmartPointer<TIn> image)
	{
		typedef itk::RescaleIntensityImageFilter<TIn, TOut> RescalerType;
		RescalerType::Pointer rescaleOutput = RescalerType::New();
		rescaleOutput->SetInput(image);
		rescaleOutput->Update();
		return rescaleOutput->GetOutput();
	}

	template <typename TIn>
	inline vtkSmartPointer<vtkImageData> itkToVtk(itk::SmartPointer<TIn> image)
	{
		using FilterType = itk::ImageToVTKImageFilter<TIn>;
		FilterType::Pointer filter = FilterType::New();
		filter->SetInput(image);

		try
		{
			filter->Update();
		}
		catch (itk::ExceptionObject & error)
		{
			std::cerr << error << std::endl;
			return nullptr;
		}

		// TODO ?
		//vtkSmartPointer<vtkImageData> result = vtkSmartPointer<vtkImageData>::New();
		//result->ShallowCopy(filter->GetOutput());
		//return result;

		return filter->GetOutput();
	}

	template <typename TOut>
	inline itk::SmartPointer<TOut> vtkToItk(vtkSmartPointer<vtkImageData> vtkImage)
	{
		typedef itk::VTKImageToImageFilter<TOut> ConverterType;
		ConverterType::Pointer converter = ConverterType::New();
		converter->SetInput(vtkImage);
		converter->Update();

		/*const itk::VTKImageToImageFilter<TOut>::OutputImageType *result = converter->GetOutput();
		typedef itk::CastImageFilter<itk::VTKImageToImageFilter<TOut>::OutputImageType, TOut> CastOutputType;*/

		typedef itk::CastImageFilter<itk::VTKImageToImageFilter<TOut>::OutputImageType, TOut> CastOutputType;
		CastOutputType::Pointer castOutput = CastOutputType::New();
		castOutput->SetInput(converter->GetOutput());
		castOutput->Update();
		return castOutput->GetOutput();
	}

	// vtk - vtk conversion
	template <typename TIn, typename TOut>
	vtkSmartPointer<vtkImageData> ConvertVtk(vtkSmartPointer<vtkImageData> vtkImage)
	{
		// convert to itk, then to itk float and back to vtk
		auto itk = vtkToItk<TIn>(vtkImage);
		auto itkConv = Convert<TIn, TOut>(itk);
		return itkToVtk<TOut>(itkConv);
	}
}