#pragma once
#include "DicomSeries.h"
#include "ITK-5.1/itkNeighborhoodIterator.h"
#include "ITK-5.1/itkImageToImageFilter.h"
#include "ITK-5.1/itkImageRegionIteratorWithIndex.h"
#include "ITK-5.1/itkConstantBoundaryCondition.h"

class BinaryThinningImageFilter3D :
	public itk::ImageToImageFilter<DicomSeriesType, DicomSeriesType>
{
public:
	ITK_DISALLOW_COPY_AND_ASSIGN(BinaryThinningImageFilter3D);

	/** Standard class typedefs. */
	typedef BinaryThinningImageFilter3D    Self;
	typedef itk::ImageToImageFilter<DicomSeriesType, DicomSeriesType> Superclass;
	typedef itk::SmartPointer<Self> Pointer;
	typedef itk::SmartPointer<const Self> ConstPointer;

	/** Method for creation through the object factory */
	itkNewMacro(BinaryThinningImageFilter3D);

	BinaryThinningImageFilter3D();
	~BinaryThinningImageFilter3D() {};

	/** Run-time type information (and related methods). */
	itkTypeMacro(BinaryThinningImageFilter3D, ImageToImageFilter);

	/** Type for the region of the input image. */
	typedef typename InputImageType::RegionType RegionType;

	/** Type for the index of the input image. */
	typedef typename RegionType::IndexType  IndexType;

	/** Type for the pixel type of the input image. */
	typedef typename InputImageType::PixelType InputImagePixelType;

	/** Type for the pixel type of the input image. */
	typedef typename OutputImageType::PixelType OutputImagePixelType;

	/** Type for the size of the input image. */
	typedef typename RegionType::SizeType SizeType;

	/** Pointer Type for input image. */
	typedef typename InputImageType::ConstPointer InputImagePointer;

	/** Pointer Type for the output image. */
	typedef typename OutputImageType::Pointer OutputImagePointer;

	/** Boundary condition type for the neighborhood iterator */
	typedef itk::ConstantBoundaryCondition<DicomSeriesType> ConstBoundaryConditionType;

	/** Neighborhood iterator type */
	typedef itk::NeighborhoodIterator<DicomSeriesType, ConstBoundaryConditionType> NeighborhoodIteratorType;

	/** Neighborhood type */
	typedef typename NeighborhoodIteratorType::NeighborhoodType NeighborhoodType;

	/** Get Skelenton by thinning image. */
	DicomSeriesType::Pointer GetThinning(void);

protected:
	//void PrintSelf(std::ostream& os, Indent indent) const;

	/** Compute thinning Image. */
	void GenerateData();

	/** Prepare data. */
	void PrepareData();

	/**  Compute thinning Image. */
	void ComputeThinImage();

	/**  isEulerInvariant [Lee94] */
	bool isEulerInvariant(NeighborhoodType neighbors, int *LUT);
	void fillEulerLUT(int *LUT);
	/**  isSimplePoint [Lee94] */
	bool isSimplePoint(NeighborhoodType neighbors);
	/**  Octree_labeling [Lee94] */
	void Octree_labeling(int octant, int label, int *cube);

private:
	DicomSeriesType::PixelType _foregroundValue = 1;
	DicomSeriesType::PixelType _backgroundValue = 0;

};

