#pragma once
#include "ITK-5.1/itkImage.h"
#include "ITK-5.1/itkImageFileReader.h"
#include "ITK-5.1/itkImageFileWriter.h"
#include "ITK-5.1/itkRGBPixel.h"
#include "ITK-5.1/itkImageSeriesReader.h"
#include "ITK-5.1/itkGDCMImageIO.h"

//#define ITK_VERSION "5.1.0"

// rgb
typedef itk::Image<itk::RGBPixel<unsigned char>, 2> RgbImageType;

// grayscale
typedef itk::Image<unsigned char, 2> GrayImageType;

// dicom image
typedef itk::Image<signed short, 2> DicomImageType;
typedef itk::Image<float, 2> DicomImageFloatType;

// dicom series
typedef itk::Image<signed short, 3> DicomSeriesType;
typedef itk::Image<float, 3> DicomSeriesFloatType;
