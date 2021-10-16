#include "MinimalCrossSectionFinder.h"
#include "SkeletonToGraphFilter.h"
#include "PointsToLineLeastSquaresAlgorithm3D.h"
#include "LaplacianPointsSmoothener.h"

//#include <vtkSmartPointer.h>
//#include <vtkIdList.h>
//#include <vtkKdTree.h>
//#include <vtkPoints.h>
//#include <vtkPolyData.h>
//#include <vtkVertexGlyphFilter.h>

void MinimalCrossSectionFinder::Compute()
{
	ComputeUsingLocalGraph();
}

void MinimalCrossSectionFinder::ComputeUsingLocalGraph()
{
	// create graph
	auto graph = CreateMinimalPathsGraph();
	auto vertices = graph->GetVertices();
	std::cout << "[MinimalCrossSection] Vertex count: " << graph->GetVerticesCount() << std::endl;
	for (int i = 0; i < vertices.size(); i++)
	{
		auto vertex = vertices[i];
		auto in = vertex.index;
		std::cout << vertex.id << ": ";
		std::cout << in[0] << ", " << in[1] << ", " << in[2];
		std::cout << " deg = " << vertex.degree << std::endl;
	}


	// get start and end vertex
	int minZ = INT_MAX;
	Vertex<IndexType> start, end;
	for (int i = 0; i < vertices.size(); i++)
	{
		auto vertex = vertices[i];
		if (vertex.degree == 3) end = vertex;

		if (start.id == -1)
		{
			minZ = vertex.index[2];
			start = vertex;
		}
		else if (vertex.index[2] < minZ)
		{
			minZ = vertex.index[2];
			start = vertex;
		}
	}

	// dump info
	bool areNeighbors = false;
	auto adjacent = graph->GetAdjacentVertices(start.id);
	for (int i = 0; i < adjacent.size(); i++)
		if (adjacent[i] == end.id) areNeighbors = true;

	std::cout << "[MinimalCrossSection] Start vertex: ID = " << start.id;
	std::cout << " Location: " << start.index[0] << ", " << start.index[1] << ", " << start.index[2] << std::endl;

	std::cout << "[MinimalCrossSection] End vertex: ID = " << end.id;
	std::cout << " Location: " << end.index[0] << ", " << end.index[1] << ", " << end.index[2] << std::endl;

	// get all indexes between start and end
	auto edge = graph->GetEdgeIndexes(start.id, end.id);
	auto allPoints = std::vector<IndexType>();
	allPoints.push_back(start.index);
	for (int i = 0; i < edge.size(); i++)
		allPoints.push_back(edge[i]);
	allPoints.push_back(end.index);

	// line fitting
	auto fit = new PointsToLineLeastSquaresAlgorithm3D();
	fit->SetPoints(allPoints);
	fit->Compute();
	fittedLine = fit->GetLine();
	std::cout << "[MinimalCrossSection] Fitted line: P0 = "
		<< fittedLine[0] << ", " << fittedLine[1] << ", " << fittedLine[2] << " k = "
		<< fittedLine[3] << ", " << fittedLine[4] << ", " << fittedLine[5] << std::endl;

	std::cout << "[MinimalCrossSection] Calculating cross section from " << allPoints.size() << " points." << std::endl;

	// set split plane point in order to look for normal while calculating stuff
	splitPlanePoint = end.index;

	//CalculateMinimalCrossSection(allPoints);
	//CalculateMinimalCrossSection2(allPoints);
	//CalculateMinimalCrossSection3(allPoints);
	//CalculateMinimalCrossSection4(allPoints);
	//CalculateMinimalCrossSection5(allPoints);
	CalculateMinimalCrossSection6(allPoints);

	// set end (degree == 3) as split point for nasal cavity and pharynx
	std::cout << "[MinimalCrossSection] Preparing split plane... ";
	std::cout << "P: [" << splitPlanePoint[0] << "," << splitPlanePoint[1] << "," << splitPlanePoint[2] << "] ";
	std::cout << "N: [" << splitPlaneNormal[0] << "," << splitPlaneNormal[1] << "," << splitPlaneNormal[2] << "] " << std::endl;

	delete fit;
	delete graph;
}

void MinimalCrossSectionFinder::CalculateMinimalCrossSection(std::vector<IndexType> points)
{
	// cross sections perpendicular to fitted line
	std::vector<double> normal(fittedLine.begin() + 3, fittedLine.end());

	minimalSections = CalculateCrossSections(points, normal);
	double minimalSection = DBL_MAX;
	double previous = minimalSections[0];
	IndexType minimalPoint;

	bool startChecking = false;
	bool found = false;
	for (int i = 1; i < points.size(); i++)
	{
		auto in = points[i];
		auto section = minimalSections[i];

		if (!startChecking)
		{
			startChecking = section < previous;
			previous = section;
		}
		else if (section < minimalSection && section != 0)
		{
			found = true;
			minimalSection = section;
			minimalPoint = in;
		}
	}

	calculatedSection = found ? minimalSection : -1;
	calculatedSectionPoint = minimalPoint;
	calculatedSectionNormal = normal;

	std::cout << "[MinimalCrossSection] Method 1 = " << calculatedSection << " mm2 in point: "
		<< calculatedSectionPoint[0] << ", " << calculatedSectionPoint[1] << ", " << calculatedSectionPoint[2] << std::endl;
}

void MinimalCrossSectionFinder::CalculateMinimalCrossSection2(std::vector<IndexType> points)
{
	// cross sections perpendicular to locally fitted line
	auto normals = CalculateLocalNormals(points);
	minimalSections2 = CalculateCrossSections2(points, normals);

	double minimalSection = DBL_MAX;
	double previous = minimalSections2[0];
	IndexType minimalPoint;

	int minIndex = 0;
	bool startChecking = false;
	bool found = false;
	for (int i = 1; i < points.size(); i++)
	{
		auto in = points[i];
		auto section = minimalSections2[i];

		if (!startChecking)
		{
			startChecking = section < previous;
			previous = section;
		}
		else if (section < minimalSection && section != 0)
		{
			found = true;
			minimalSection = section;
			minimalPoint = in;
			minIndex = i;
		}
	}

	calculatedSection2 = found ? minimalSection : -1;
	calculatedSectionPoint2 = minimalPoint;
	calculatedSectionNormal2 = normals[minIndex];

	std::cout << "[MinimalCrossSection] Method 2 = " << calculatedSection2 << " mm2 in point: "
		<< calculatedSectionPoint2[0] << ", " << calculatedSectionPoint2[1] << ", " << calculatedSectionPoint2[2] << std::endl;
}

void MinimalCrossSectionFinder::CalculateMinimalCrossSection3(std::vector<IndexType> points)
{
	// cross sections perpendicular to fitted line, with normal [0, y, 1]
	std::vector<double> normal(fittedLine.begin() + 3, fittedLine.end());
	normal[0] = 0;

	minimalSections3 = CalculateCrossSections(points, normal);
	double minimalSection = DBL_MAX;
	double previous = minimalSections3[0];
	IndexType minimalPoint;

	bool startChecking = false;
	bool found = false;
	for (int i = 1; i < points.size(); i++)
	{
		auto in = points[i];
		auto section = minimalSections3[i];

		if (!startChecking)
		{
			startChecking = section < previous;
			previous = section;
		}
		else if (section < minimalSection && section != 0)
		{
			found = true;
			minimalSection = section;
			minimalPoint = in;
		}
	}

	calculatedSection3 = found ? minimalSection : -1;
	calculatedSectionPoint3 = minimalPoint;
	calculatedSectionNormal3 = normal;

	std::cout << "[MinimalCrossSection] Method 3 = " << calculatedSection3 << " mm2 in point: "
		<< calculatedSectionPoint3[0] << ", " << calculatedSectionPoint3[1] << ", " << calculatedSectionPoint3[2] << std::endl;
}

void MinimalCrossSectionFinder::CalculateMinimalCrossSection4(std::vector<IndexType> points)
{
	// cross sections perpendicular to locally fitted lines, with normal [0, y, 1]
	auto initNormals = CalculateLocalNormals(points);
	for (int i = 0; i < initNormals.size(); i++)
		initNormals[i][0] = 0;

	// smoothen normals
	std::vector<std::vector<double>> normals;
	normals.resize(initNormals.size());
	normals[0] = initNormals[0];
	if (normals[0][1] > 1)
		normals[0][1] = 1;
	else if (normals[0][1] < -1)
		normals[0][1] = -1;
	for (int i = 1; i < initNormals.size() - 1; i++)
	{
		normals[i] = initNormals[i];
		//if (std::abs(initNormals[i][1] - initNormals[i - 1][1]) >= 2)
		//	normals[i][1] = (initNormals[i + 1][1] + initNormals[i - 1][1]) / 2;
		if (normals[i][1] > 1)
			normals[i][1] = 1;
		else if (normals[i][1] < -1)
			normals[i][1] = -1;
	}
	auto last = initNormals.size() - 1;
	normals[last] = initNormals[last];
	if (normals[last][1] > 1)
		normals[last][1] = 1;
	else if (normals[0][1] < -1)
		normals[last][1] = -1;

	// calculate cross sections
	minimalSections4 = CalculateCrossSections2(points, normals);

	// skip some part of first and last points
	auto toSkip = 5;
	double previous = minimalSections4[toSkip];
	minimalSections4 = std::vector<double>(minimalSections4.begin() + toSkip, minimalSections4.end() - toSkip);
	normals = std::vector<std::vector<double>>(normals.begin() + toSkip, normals.end() - toSkip);
	auto localPoints = std::vector<IndexType>(points.begin() + toSkip, points.end() - toSkip);
	auto beforeSize = minimalSections4.size();

	double minimalSection = DBL_MAX;
	IndexType minimalPoint;

	bool startChecking = false;
	bool found = false;
	int minIndex = 0;
	int additionalSkipped = 0;
	for (int i = toSkip; i < localPoints.size() - toSkip; i++)
	{
		auto in = localPoints[i];
		auto section = minimalSections4[i];

		if (!startChecking)
		{
			additionalSkipped++;
			startChecking = section < previous;
			previous = section;
		}
		else if (section < minimalSection && section != 0)
		{
			found = true;
			minimalSection = section;
			minimalPoint = in;
			minIndex = i;
		}
	}

	// remove those which were skipped as well, because of "startChecking"
	minimalSections4 = std::vector<double>(minimalSections4.begin() + additionalSkipped, minimalSections4.end());
	std::cout << "[MinimalCrossSection] Sanity count check. Points: " << points.size() << " Cut: "
		<< localPoints.size() << " Sections: " << beforeSize << " After: " << minimalSections4.size() << std::endl;

	calculatedSection4 = found ? minimalSection : -1;
	calculatedSectionPoint4 = minimalPoint;
	calculatedSectionNormal4 = normals[minIndex];

	std::cout << "[MinimalCrossSection] Method 4 = " << calculatedSection4 << " mm2 in point: "
		<< calculatedSectionPoint4[0] << ", " << calculatedSectionPoint4[1] << ", " << calculatedSectionPoint4[2] << std::endl;
}

void MinimalCrossSectionFinder::CalculateMinimalCrossSection5(std::vector<IndexType> points)
{
	auto initNormals = CalculateLocalNormals(points);

	// limit X direction normal to 2
	std::vector<std::vector<double>> normals;
	normals.resize(initNormals.size());
	for (int i = 0; i < initNormals.size(); i++)
	{
		normals[i] = initNormals[i];
		if (normals[i][0] > 1)
			normals[i][0] = 1;
		else if (normals[i][0] < -1)
			normals[i][0] = 1;
	}

	// calculate cross sections
	minimalSections4 = CalculateCrossSections3(points, normals);

	// skip some part of first and last points
	auto toSkip = 0;
	double previous = minimalSections4[toSkip];
	minimalSections4 = std::vector<double>(minimalSections4.begin() + toSkip, minimalSections4.end() - toSkip);
	normals = std::vector<std::vector<double>>(normals.begin() + toSkip, normals.end() - toSkip);
	auto localPoints = std::vector<IndexType>(points.begin() + toSkip, points.end() - toSkip);
	auto beforeSize = minimalSections4.size();

	double minimalSection = DBL_MAX;
	IndexType minimalPoint;

	bool startChecking = false;
	bool found = false;
	int minIndex = 0;
	int additionalSkipped = 0;
	int additionalSkipped2 = 0;
	for (int i = toSkip; i < localPoints.size() - toSkip; i++)
	{
		auto in = localPoints[i];
		auto section = minimalSections4[i];

		if (!startChecking)
		{
			additionalSkipped++;
			startChecking = section < previous;
			previous = section;
		}
		else if (normals[i][1] < -1.5)
		{
			additionalSkipped2 = localPoints.size() - toSkip - i;
			break;
		}
		else if (section < minimalSection && section != 0)
		{
			found = true;
			minimalSection = section;
			minimalPoint = in;
			minIndex = i;
		}
	}

	// remove those which were skipped as well, because of "startChecking"
	minimalSections4 = std::vector<double>(minimalSections4.begin() + additionalSkipped, minimalSections4.end() - additionalSkipped2);
	localPoints = std::vector<IndexType>(localPoints.begin() + additionalSkipped, localPoints.end() - additionalSkipped2);
	normals = std::vector<std::vector<double>>(normals.begin() + additionalSkipped, normals.end() - additionalSkipped2);
	std::cout << "[MinimalCrossSection] Sanity count check. Points: " << points.size() << " Cut: "
		<< localPoints.size() << " Sections: " << beforeSize << " After: " << minimalSections4.size() << std::endl;

	minimalCrossSectionsCoords = localPoints;
	minimalCrossSectionsNormals = normals;

	calculatedSection4 = found ? minimalSection : -1;
	calculatedSectionPoint4 = minimalPoint;
	calculatedSectionNormal4 = normals[minIndex];

	std::cout << "[MinimalCrossSection] Method 5 = " << calculatedSection4 << " mm2 in point: "
		<< calculatedSectionPoint4[0] << ", " << calculatedSectionPoint4[1] << ", " << calculatedSectionPoint4[2] << std::endl;
}

void MinimalCrossSectionFinder::CalculateMinimalCrossSection6(std::vector<IndexType> points)
{
	auto initNormals = CalculateLocalNormals(points);

	// limit X direction normal to 1
	std::vector<std::vector<double>> normals;
	normals.resize(initNormals.size());
	for (int i = 0; i < initNormals.size(); i++)
	{
		normals[i] = initNormals[i];
		if (normals[i][0] > 1)
			normals[i][0] = 1;
		else if (normals[i][0] < -1)
			normals[i][0] = 1;
	}

	// calculate cross sections
	minimalSections4 = CalculateCrossSections4(points, normals);

	// skip some part of first and last points
	auto toSkip = 0;
	double previous = minimalSections4[toSkip];
	//minimalSections4 = std::vector<double>(minimalSections4.begin() + toSkip, minimalSections4.end() - toSkip);
	//normals = std::vector<std::vector<double>>(normals.begin() + toSkip, normals.end() - toSkip);
	auto localPoints = points; //std::vector<IndexType>(points.begin() + toSkip, points.end() - toSkip);
	auto beforeSize = minimalSections4.size();

	double minimalSection = DBL_MAX;
	IndexType minimalPoint;

	bool startChecking = false;
	bool found = false;
	int minIndex = 0;
	int additionalSkipped = 0;
	int additionalSkipped2 = 0;
	for (int i = toSkip; i < localPoints.size() - toSkip; i++)
	{
		auto in = localPoints[i];
		auto section = minimalSections4[i];

		if (!startChecking)
		{
			additionalSkipped++;
			startChecking = section < previous;
			previous = section;
		}
		else if (normals[i][1] < -1.25)
		{
			additionalSkipped2 = localPoints.size() - toSkip - i;
			break;
		}
		else if (section < minimalSection && section != 0)
		{
			found = true;
			minimalSection = section;
			minimalPoint = in;
			minIndex = i;
		}
	}

	// override split plane normal with minimal Y from skipped part
	int minVectorIn = minimalSections4.size() - additionalSkipped - additionalSkipped2;
	for (int i = minVectorIn; i < normals.size(); i++)
	{
		if (normals[i][1] < normals[minVectorIn][1])
		{
			minVectorIn = i;
		}
	}
	splitPlaneNormal = normals[minVectorIn];
	splitPlaneNormal[0] = 0;

	// remove those which were skipped as well, because of "startChecking"
	minimalSections4 = std::vector<double>(minimalSections4.begin() + additionalSkipped, minimalSections4.end() - additionalSkipped2);
	localPoints = std::vector<IndexType>(localPoints.begin() + additionalSkipped, localPoints.end() - additionalSkipped2);
	normals = std::vector<std::vector<double>>(normals.begin() + additionalSkipped, normals.end() - additionalSkipped2);
	std::cout << "[MinimalCrossSection] Sanity count check. Points: " << points.size() << " Cut: "
		<< localPoints.size() << " Sections: " << beforeSize << " After: " << minimalSections4.size() << " MinIndex: " << minIndex << std::endl;

	minimalCrossSectionsCoords = localPoints;
	minimalCrossSectionsNormals = normals;

	calculatedSection4 = found ? minimalSection : -1;
	calculatedSectionPoint4 = minimalPoint;
	calculatedSectionNormal4 = normals[minIndex - additionalSkipped];

	std::cout << "[MinimalCrossSection] Method 6 = " << calculatedSection4 << " mm2 in point: "
		<< calculatedSectionPoint4[0] << ", " << calculatedSectionPoint4[1] << ", " << calculatedSectionPoint4[2] << std::endl;
}

std::vector<std::vector<double>>
MinimalCrossSectionFinder::CalculateLocalNormals(
	std::vector<IndexType> anchorPoints)
{
	NeighborhoodIteratorType::RadiusType radius;
	for (int i = 0; i < ImageType::ImageDimension; i++) radius[i] = 7;
	auto nit = NeighborhoodIteratorType(radius, minimalPathsImage, minimalPathsImage->GetLargestPossibleRegion());

	// fit normals
	std::vector<std::vector<double>> normals;
	for (int i = 0; i < anchorPoints.size(); i++)
	{
		auto pt = anchorPoints[i];
		nit.SetLocation(pt);

		std::vector<IndexType> neighbors;
		for (int j = 0; j < nit.Size(); j++)
		{
			if (nit.GetPixel(j) == SHRT_MAX)
			{
				auto in = nit.GetIndex(j);
				if (std::find(anchorPoints.begin(), anchorPoints.end(), in) != anchorPoints.end())
					neighbors.push_back(in);
			}
		}

		std::vector<double> normal = std::vector<double>{ 0, 0, 0 };
		if (neighbors.size() >= 3)
		{
			auto lineFitter = PointsToLineLeastSquaresAlgorithm3D::New();
			lineFitter->SetPoints(neighbors);
			lineFitter->Compute(true);
			auto fit = lineFitter->GetLine();
			if (!fit.empty())
				normal = std::vector<double>(fit.begin() + 3, fit.end());
		}

		normals.push_back(normal);
		if (pt == splitPlanePoint)
		{
			splitPlaneNormal = normal;
			splitPlaneNormal[0] = 0;
		}
	}

	// force normals for first few and last few points
	for (int i = 0; i < 5; i++)
	{
		normals[i][0] = 0;
		normals[i][1] = 0;

		normals[normals.size() - 1 - i][0] = 0;
	}

	auto smoothener = new LaplacianPointsSmoothener();
	normals = smoothener->SmoothenPoints(normals, true, 20, false);
	return normals;
}

std::vector<double> MinimalCrossSectionFinder::CalculateCrossSections(
	std::vector<IndexType> anchorPoints, std::vector<double> normal)
{	
	std::vector<std::vector<IndexType>> crossSectionsVoxels;
	crossSectionsVoxels.resize(anchorPoints.size());
	
	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto iterator = ConstIteratorType(image, image->GetLargestPossibleRegion());
	iterator.GoToBegin();

	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == SHRT_MAX)
		{
			for (int i = 0; i < anchorPoints.size(); i++)
			{
				auto in = iterator.GetIndex();
				if (IsOnPlane(in, anchorPoints[i], normal))
					crossSectionsVoxels[i].push_back(in);
			}
		}
		++iterator;
	}

	// TODO - filter indexes so that they create one group, not more
	//for (int i = 0; i < anchorPoints.size(); i++)
	//	crossSectionsVoxels[i] = FilterCrossSection(crossSectionsVoxels[i], anchorPoints[i], i);

	// TODO - calculate based on normal angle?
	std::vector<double> crossSections;
	auto spacing = image->GetSpacing();
	auto normalized = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	auto cosalpha = normalized == 0 ? 0 : normal[2] / normalized;
	std::cout << "[MinimalCrossSection] Cos alpha: " << cosalpha << std::endl;
	for (int i = 0; i < crossSectionsVoxels.size(); i++)
	{
		double val = std::abs(cosalpha) * crossSectionsVoxels[i].size();
		val *= spacing[0] * spacing[1];
		crossSections.push_back(val);
		std::cout << i << ": " << val << std::endl;
	}
	return crossSections;
}

std::vector<MinimalCrossSectionFinder::IndexType>
MinimalCrossSectionFinder::FilterCrossSection(
	std::vector<IndexType> voxels, IndexType anchorPoint, int index)
{
	signed short currentId = 1;
	image->SetPixel(anchorPoint, currentId);

	NeighborhoodIteratorType::RadiusType radius;
	for (int i = 0; i < ImageType::ImageDimension; i++) radius[i] = 1;
	auto nit = NeighborhoodIteratorType(radius, image, image->GetLargestPossibleRegion());

	// bottom up
	for (int i = 0; i < voxels.size(); i++)
	{
		auto in = voxels[i];
		nit.SetLocation(in);
		bool found = false;
		for (int n = 0; n < nit.Size(); n++)
		{
			auto px = nit.GetPixel(n);
			if (px != SHRT_MAX && px < currentId)
			{
				found = true;
				nit.SetCenterPixel(px);
				break;
			}
		}

		if (!found)
		{
			currentId++;
			nit.SetCenterPixel(currentId);
		}
	}
	
	// top down
	for (int i = voxels.size() - 1; i >= 0; i--)
	{
		auto in = voxels[i];
		nit.SetLocation(in);
		bool found = false;
		for (int n = 0; n < nit.Size(); n++)
		{
			auto px = nit.GetPixel(n);
			if (px != SHRT_MAX && px < currentId)
			{
				found = true;
				nit.SetCenterPixel(px);
				break;
			}
		}

		if (!found)
		{
			currentId++;
			nit.SetCenterPixel(currentId);
		}
	}

	auto finalId = image->GetPixel(anchorPoint);
	std::vector<IndexType> finalGroup;
	std::vector<int> counter;
	counter.resize(currentId);
	for (int i = 0; i < voxels.size(); i++)
	{
		auto px = image->GetPixel(voxels[i]);
		if (px <= currentId)
		{
			counter[currentId - 1]++;
			if (px == finalId)
				finalGroup.push_back(voxels[i]);
		}
	}

	int groupCount = 0;
	for (int i = 0; i < counter.size(); i++)
		if (counter[i] > 0) groupCount++;

std::cout << "[MinimalCrossSection] " << index << " IDs: " << currentId
<< " Groups: " << groupCount << std::endl;

// revert
for (int i = 0; i < voxels.size(); i++)
	image->SetPixel(voxels[i], SHRT_MAX);

return finalGroup;
}

std::vector<double> MinimalCrossSectionFinder::CalculateCrossSections2(
	std::vector<IndexType> anchorPoints, std::vector<std::vector<double>> normals)
{
	std::vector<std::vector<IndexType>> crossSectionsVoxels;
	crossSectionsVoxels.resize(anchorPoints.size());

	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto iterator = ConstIteratorType(image, image->GetLargestPossibleRegion());
	iterator.GoToBegin();

	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == SHRT_MAX)
		{
			for (int i = 0; i < anchorPoints.size(); i++)
			{
				auto in = iterator.GetIndex();
				if (IsOnPlane(in, anchorPoints[i], normals[i]))
					crossSectionsVoxels[i].push_back(in);
			}
		}
		++iterator;
	}

	//TODO - filter indexes so that they create one group, not more
	//std::cout << "[MinimalCrossSection] Filtering..." << std::endl;
	//for (int i = 0; i < anchorPoints.size(); i++)
	//	crossSectionsVoxels[i] = FilterCrossSection2(crossSectionsVoxels[i], anchorPoints[i], i);

	// TODO - calculate based on normal angle?
	std::vector<double> crossSections;
	auto spacing = image->GetSpacing();
	for (int i = 0; i < crossSectionsVoxels.size(); i++)
	{
		auto normalized = std::sqrt(normals[i][0] * normals[i][0] + normals[i][1] * normals[i][1] + normals[i][2] * normals[i][2]);
		auto cosalpha = normalized == 0 ? 0 : normals[i][2] / normalized;

		double val = std::abs(cosalpha) * crossSectionsVoxels[i].size();
		val *= spacing[0] * spacing[1];
		crossSections.push_back(val);
		std::cout << i << ": " << val << " N: " << normals[i][0] << "," << normals[i][1] << "," << normals[i][2] << " Cos: " << cosalpha << std::endl;
	}
	return crossSections;
}

std::vector<double> MinimalCrossSectionFinder::CalculateCrossSections3(
	std::vector<IndexType> anchorPoints, std::vector<std::vector<double>> normals)
{
	std::vector<std::vector<IndexType>> crossSectionsVoxels;
	crossSectionsVoxels.resize(anchorPoints.size());

	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto iterator = ConstIteratorType(image, image->GetLargestPossibleRegion());
	iterator.GoToBegin();

	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == SHRT_MAX)
		{
			for (int i = 0; i < anchorPoints.size(); i++)
			{
				auto in = iterator.GetIndex();
				if (IsOnPlane2(in, anchorPoints[i], normals[i])
					&& std::abs(in[1] - anchorPoints[i][1]) < 30
					&& std::abs(in[2] - anchorPoints[i][2]) < 30
					&& std::abs(in[0] - anchorPoints[i][0]) < 40)
				{
					crossSectionsVoxels[i].push_back(in);
				}
			}
		}
		++iterator;
	}


	// calculate ellipse area
	std::vector<double> crossSections;
	auto spacing = image->GetSpacing();
	for (int i = 0; i < crossSectionsVoxels.size(); i++)
	{
		auto boundaries = GetBoundaryVoxels(crossSectionsVoxels[i]);
		double val = GetEllipseArea(boundaries);
		crossSections.push_back(val);
		std::cout << i << ":\t" << val << " N: " << normals[i][0] << ",\t" << normals[i][1] << ",\t" << normals[i][2]
			<< "\tVoxels before:\t" << crossSectionsVoxels[i].size() << "\tAfter:\t" << boundaries.size() << std::endl;
	}
	return crossSections;
}

std::vector<MinimalCrossSectionFinder::IndexType>
MinimalCrossSectionFinder::GetBoundaryVoxels(std::vector<IndexType> voxels)
{
	NeighborhoodIteratorType::RadiusType radius;
	for (int i = 0; i < ImageType::ImageDimension; i++) radius[i] = 1;
	auto nit = NeighborhoodIteratorType(radius, image, image->GetLargestPossibleRegion());

	std::vector<IndexType> boundaries;
	for (int i = 0; i < voxels.size(); i++)
	{
		nit.SetLocation(voxels[i]);
		bool isFull = true;
		for (int i = 0; i < nit.Size(); i++)
		{
			if (nit.GetPixel(i) != SHRT_MAX)
			{
				isFull = false;
				break;
			}
		}

		if (!isFull)
			boundaries.push_back(voxels[i]);

	}

	return boundaries;
}

double MinimalCrossSectionFinder::GetEllipseArea(
	std::vector<IndexType> points)
{	
	// geometric center
	double x = 0, y = 0, z = 0;
	for (int i = 0; i < points.size(); i++)
	{
		x += points[i][0];
		y += points[i][1];
		z += points[i][2];
	}
	x /= points.size();
	y /= points.size();
	z /= points.size();

	auto sp = image->GetSpacing();
	//auto origin = image->GetOrigin();
	//auto realX = origin[0] + x * sp[0];
	//auto realY = origin[1] + x * sp[1];
	//auto realZ = origin[2] + x * sp[2];

	//auto vPoints = vtkSmartPointer<vtkPoints>::New();
	//for (int i = 0; i < points.size(); i++)
	//	vPoints->InsertNextPoint(origin[0] + sp[0] * points[i][0], origin[1] + sp[1] * points[i][1], origin[2] + sp[2] * points[i][2]);

	// TODO?
	//auto kdTree = vtkSmartPointer<vtkKdTree>::New();
	//kdTree->BuildLocatorFromPoints(vPoints);
	//double kdDistance;
	//auto id = kdTree->FindClosestPoint(realX, realY, realZ, kdDistance);
	//auto closest = kdTree->GetDataSet()->GetPoint(id);
	//kdDistance = std::sqrt(
	//	std::pow((x - closest[0]) * sp[0], 2) + std::pow((y - closest[1]) * sp[1], 2) + std::pow((z - closest[2]) * sp[2], 2));
	//return vtkMath::Pi() * kdDistance * kdDistance;

	IndexType maxPoint;
	IndexType minPoint;
	double maxDist = 0, minDist = DBL_MAX;
	for (int i = 0; i < points.size(); i++)
	{
		auto pt = points[i];
		auto dist = std::sqrt(
			std::pow((x - pt[0]) * sp[0], 2) + std::pow((y - pt[1]) * sp[1], 2) + std::pow((z - pt[2]) * sp[2], 2));
		if (dist > maxDist)
		{
			maxDist = dist;
			maxPoint = pt;
		}

		if (dist < minDist)
		{
			minDist = dist;
			minPoint = pt;
		}
	}

	return itk::Math::pi * maxDist * minDist;
}

std::vector<MinimalCrossSectionFinder::IndexType>
MinimalCrossSectionFinder::FilterCrossSection2(
	std::vector<IndexType> voxels, IndexType anchorPoint, int index)
{
	NeighborhoodIteratorType::RadiusType radius;
	for (int i = 0; i < ImageType::ImageDimension; i++) radius[i] = 1;
	auto nit = NeighborhoodIteratorType(radius, image, image->GetLargestPossibleRegion());

	std::vector<IndexType> finalGroup;
	for (int i = 0; i < voxels.size(); i++)
	{
		bool shouldRemove = false;

		auto in = voxels[i];
		IndexType next = in;
		nit.SetLocation(in);
		std::vector<double> normal;
		normal.push_back(anchorPoint[0] - in[0]);
		normal.push_back(anchorPoint[1] - in[1]);
		normal.push_back(anchorPoint[2] - in[2]);
		do
		{
			bool found = false;
			IndexType closest;
			double bestErr = DBL_MAX;
			for (int j = 0; j < nit.Size(); j++)
			{
				auto subIn = nit.GetIndex(j);
				if (subIn == in) continue;

				double subErr;
				IsOnLine(subIn, in, normal, &subErr);
				if (subErr < bestErr)
				{
					found = true;
					bestErr = subErr;
					closest = subIn;
				}
			}

			if (!found)
			{
				std::cout << "[MinimalCrossSection] ERROR while filtering, cannot proceed to next closest neighbor." << std::endl;
				break;
			}

			next = closest;
			if (image->GetPixel(next) != SHRT_MAX)
			{
				shouldRemove = true;
				break;
			}

		} while (next != anchorPoint);

		if (!shouldRemove)
			finalGroup.push_back(in);
	}

	std::cout << "[MinimalCrossSection] Initial voxels: " << voxels.size() << " After filter: " << finalGroup.size() << std::endl;

	return finalGroup;
}


bool MinimalCrossSectionFinder::IsOnPlane(IndexType point, IndexType anchorPoint, std::vector<double> normal)
{
	if (normal.empty()) return false;

	// plane formula: A(X-X0)+B(Y-Y0)+C(Z-Z0)=0, where normal = (A,B,C)
	auto x = normal[0] * (point[0] - anchorPoint[0]);
	auto y = normal[1] * (point[1] - anchorPoint[1]);
	auto z = normal[2] * (point[2] - anchorPoint[2]);

	auto ratio = imageSize[0] / imageSize[2];
	auto normalized = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	auto cosalpha = (normal[2] / normalized);
	//auto tolerance = 1 + (ratio * (1 - cosalpha));

	//auto fakeX = normal[0] * (point[0] + 1 - anchorPoint[0]);
	//auto fakeY = normal[1] * (point[1] + 1 - anchorPoint[1]);
	//auto fakeZ = normal[2] * (point[2] + 1 - anchorPoint[2]);
	//auto tolerance = (fakeX + fakeY + fakeZ) * std::abs(cosalpha);
	auto tolerance = 1 / cosalpha / cosalpha;

	auto val = x + y + z;
	return std::abs(val) <= tolerance;
}

bool MinimalCrossSectionFinder::IsOnPlane2(IndexType point, IndexType anchorPoint, std::vector<double> normal)
{
	if (normal.empty()) return false;

	// plane formula: A(X-X0)+B(Y-Y0)+C(Z-Z0)=0, where normal = (A,B,C)
	auto x = normal[0] * (point[0] - anchorPoint[0]);
	auto y = normal[1] * (point[1] - anchorPoint[1]);
	auto z = normal[2] * (point[2] - anchorPoint[2]);

	auto val = x + y + z;
	return std::abs(val) <= 2;
}

std::vector<double> MinimalCrossSectionFinder::CalculateCrossSections4(
	std::vector<IndexType> anchorPoints, std::vector<std::vector<double>> normals)
{
	std::vector<std::vector<IndexType>> crossSectionsVoxels;
	crossSectionsVoxels.resize(anchorPoints.size());

	using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;
	auto iterator = ConstIteratorType(image, image->GetLargestPossibleRegion());
	iterator.GoToBegin();

	while (!iterator.IsAtEnd())
	{
		if (iterator.Value() == SHRT_MAX)
		{
			for (int i = 0; i < anchorPoints.size(); i++)
			{
				auto in = iterator.GetIndex();
				if (IsOnPlane3(in, anchorPoints[i], normals[i])
					&& std::abs(in[1] - anchorPoints[i][1]) < 30
					&& std::abs(in[2] - anchorPoints[i][2]) < 30
					&& std::abs(in[0] - anchorPoints[i][0]) < 40)
				{
					crossSectionsVoxels[i].push_back(in);
				}
			}
		}
		++iterator;
	}

	std::vector<double> crossSections;
	auto spacing = image->GetSpacing();
	for (int i = 0; i < crossSectionsVoxels.size(); i++)
	{
		double sum = 0;
		for (int j = 0; j < crossSectionsVoxels[i].size(); j++)
			sum += VoxelCrossSectionRealArea(crossSectionsVoxels[i][j], anchorPoints[i], normals[i], spacing);
		crossSections.push_back(sum);
		std::cout << i << ":\t" << sum << " N: " << normals[i][0] << ",\t" << normals[i][1] << ",\t" << normals[i][2] << std::endl;
	}
	return crossSections;
}

bool MinimalCrossSectionFinder::IsOnPlane3(IndexType point, IndexType anchorPoint, std::vector<double> normal)
{
	if (normal.empty()) return false;
	auto dist = PointToPlaceDist(point, anchorPoint, normal);
	return dist <= 0.70710678118; // std::sqrt(2) / 2;
}


double MinimalCrossSectionFinder::PointToPlaceDist(
	IndexType point, IndexType anchorPoint, std::vector<double> normal)
{
	auto denom = std::sqrt(
		std::pow(normal[0], 2) +
		std::pow(normal[1], 2) +
		std::pow(normal[2], 2));

	if (denom == 0) return NAN;
	
	auto d = (-1) * (normal[0] * anchorPoint[0] + normal[1] * anchorPoint[1] + normal[2] * anchorPoint[2]);
	auto nom = std::abs(normal[0] * point[0]
		+ normal[1] * point[1] + normal[2] * point[2] + d);

	return nom / denom;
}

double MinimalCrossSectionFinder::PointToPointDistXY(double* a, double* b)
{
	return std::sqrt(std::pow(a[1] - b[1], 2) + std::pow(a[0] - b[0], 2));
}

double MinimalCrossSectionFinder::VoxelCrossSectionRealArea(
	IndexType point, IndexType anchor, std::vector<double> normal, itk::Vector<double, 3U> spacing)
{
	if (normal.empty() || normal[2] == 0) return 0;

	double dx = 1; // phx
	double dy = 1; // phx

	double P = dx * dy * spacing[0] * spacing[1]; // area projected into XY

	double centerX = point[0], centerY = point[1], centerZ = point[2];
	double x1 = centerX - 0.5, x2 = centerX + 0.5; // x bounds
	double y1 = centerY - 0.5, y2 = centerY + 0.5; // y bounds
	double z1 = centerZ - 0.5, z2 = centerZ + 0.5; // z bounds

	// A(x - x0) + B(y - y0) + C(z - z0) = 0
	// z = (A(x0 - x) + B(y0 - y)/C + z0
	// y = (A(x0 - x) + C(z0 - z)/B + y0
	// x = (B(y0 - y) + C(z0 - z)/A + x0
	double z11 = normal[2] == 0 ? centerZ + 2 : (normal[0] * (anchor[0] - x1) + normal[1] * (anchor[1] - y1)) / normal[2] + anchor[2];
	double z12 = normal[2] == 0 ? centerZ + 2 : (normal[0] * (anchor[0] - x1) + normal[1] * (anchor[1] - y2)) / normal[2] + anchor[2];
	double z21 = normal[2] == 0 ? centerZ + 2 : (normal[0] * (anchor[0] - x2) + normal[1] * (anchor[1] - y1)) / normal[2] + anchor[2];
	double z22 = normal[2] == 0 ? centerZ + 2 : (normal[0] * (anchor[0] - x2) + normal[1] * (anchor[1] - y2)) / normal[2] + anchor[2];
	
	double y11 = normal[1] == 0 ? centerY + 2 : (normal[0] * (anchor[0] - x1) + normal[2] * (anchor[2] - z1)) / normal[1] + anchor[1];
	double y12 = normal[1] == 0 ? centerY + 2 : (normal[0] * (anchor[0] - x1) + normal[2] * (anchor[2] - z2)) / normal[1] + anchor[1];
	double y21 = normal[1] == 0 ? centerY + 2 : (normal[0] * (anchor[0] - x2) + normal[2] * (anchor[2] - z1)) / normal[1] + anchor[1];
	double y22 = normal[1] == 0 ? centerY + 2 : (normal[0] * (anchor[0] - x2) + normal[2] * (anchor[2] - z2)) / normal[1] + anchor[1];
								     
	double x11 = normal[0] == 0 ? centerX + 2 : (normal[1] * (anchor[1] - y1) + normal[2] * (anchor[2] - z1)) / normal[0] + anchor[0];
	double x12 = normal[0] == 0 ? centerX + 2 : (normal[1] * (anchor[1] - y1) + normal[2] * (anchor[2] - z2)) / normal[0] + anchor[0];
	double x21 = normal[0] == 0 ? centerX + 2 : (normal[1] * (anchor[1] - y2) + normal[2] * (anchor[2] - z1)) / normal[0] + anchor[0];
	double x22 = normal[0] == 0 ? centerX + 2 : (normal[1] * (anchor[1] - y2) + normal[2] * (anchor[2] - z2)) / normal[0] + anchor[0];

	std::vector<double*> points;
	if (z11 > z1 && z11 < z2) points.push_back(new double[3]{ x1, y1, z11 });
	if (z12 > z1 && z12 < z2) points.push_back(new double[3]{ x1, y2, z12 });
	if (z21 > z1 && z21 < z2) points.push_back(new double[3]{ x2, y1, z21 });
	if (z22 > z1 && z22 < z2) points.push_back(new double[3]{ x2, y2, z22 });

	if (y11 > y1 && y11 < y2) points.push_back(new double[3]{ x1, y11, z1 });
	if (y12 > y1 && y12 < y2) points.push_back(new double[3]{ x1, y12, z2 });
	if (y21 > y1 && y21 < y2) points.push_back(new double[3]{ x2, y21, z1 });
	if (y22 > y1 && y22 < y2) points.push_back(new double[3]{ x2, y22, z2 });

	if (x11 > x1 && x11 < x2) points.push_back(new double[3]{ x11, y1, z1 });
	if (x12 > x1 && x12 < x2) points.push_back(new double[3]{ x12, y1, z2 });
	if (x21 > x1 && x21 < x2) points.push_back(new double[3]{ x21, y2, z1 });
	if (x22 > x1 && x22 < x2) points.push_back(new double[3]{ x22, y2, z2 });

	if (points.size() < 3 || points.size() > 6) return 0;

	if (points.size() == 3)
	{
		double d1 = PointToPointDistXY(points[0], points[1]);
		double d2 = PointToPointDistXY(points[0], points[2]);
		double d3 = PointToPointDistXY(points[1], points[2]);
		double l = (d1 + d2 + d3) / 2;
		P = std::sqrt(l * (l - d1) * (l - d2) * (l - d3)) * spacing[0] * spacing[1];
	}
	else if (points.size() == 4)
	{
		//http://home.agh.edu.pl/~rkrzyzek/zagadnienia/zagadnienie%201.pdf
		
		if (std::abs(points[0][0] - centerX) == 0.5 && std::abs(points[0][1] - centerY) == 0.5
			&& std::abs(points[1][0] - centerX) == 0.5 && std::abs(points[1][1] - centerY) == 0.5
			&& std::abs(points[2][0] - centerX) == 0.5 && std::abs(points[2][1] - centerY) == 0.5
			&& std::abs(points[3][0] - centerX) == 0.5 && std::abs(points[3][1] - centerY) == 0.5)
		{
			// P is the same
		}
		else
		{
			// TODO
			auto sorted = SortPoints(points);
			auto area = PolygonArea(sorted);
			P = area * spacing[0] * spacing[1];

			//double dy1 = (points[0][0] + points[1][0]) * (points[1][1] - points[0][1]);
			//double dy2 = (points[1][0] + points[2][0]) * (points[2][1] - points[1][1]);
			//double dy3 = (points[2][0] + points[3][0]) * (points[3][1] - points[2][1]);
			//double dy4 = (points[3][0] + points[0][0]) * (points[0][1] - points[3][1]);
			//double sum = dy1 + dy2 + dy3 + dy4;
			//P = 0.5 * std::abs(sum) * spacing[0] * spacing[1];
			//std::cout << sum << std::endl;
		}
	}
	else if (points.size() == 5 || points.size() == 6)
	{
		// TODO
		auto sorted = SortPoints(points);
		auto area = PolygonArea(points);
		P = area * spacing[0] * spacing[1];

		//double dy1 = points[1][1] - points[4][1];
		//double dy2 = points[2][1] - points[0][1];
		//double dy3 = points[3][1] - points[1][1];
		//double dy4 = points[4][1] - points[2][1];
		//double dy5 = points[0][1] - points[3][1];
		//double sum = points[0][0] * dy1 + points[1][0] * dy2 + points[2][0] * dy3 + points[3][0] * dy4 + points[4][0] * dy5;
		//P = 0.5 * std::abs(sum) * spacing[0] * spacing[1];
	}

	// surface integral
	auto I = std::abs(std::sqrt(normal[0] * normal[0] * spacing[0] * spacing[0]
		+ normal[1] * normal[1] * spacing[1] * spacing[1]
		+ normal[2] * normal[2] * spacing[2] * spacing[2]) / (normal[2] * spacing[2]));
	return P * I;
}


std::vector<double*> MinimalCrossSectionFinder::SortPoints(std::vector<double*> points)
{
	double sumX = 0, sumY = 0;
	for (int i = 0; i < points.size(); i++)
	{
		sumX += points[i][0];
		sumY += points[i][1];
	}
	double avgX = sumX / points.size() + 0.02;
	double avgY = sumY / points.size() - 0.02;

	std::vector<double> angles;
	for (int i = 0; i < points.size(); i++)
		angles.push_back(std::atan2(points[i][1] - avgY, points[i][0] - avgX));

	std::vector<int> sortedIndexes;
	for (int i = 0; i < points.size(); i++)
	{
		auto curr = angles[i];
		if (sortedIndexes.empty())
			sortedIndexes.push_back(i);
		else
		{
			int lowerThanIndex = -1;
			for (int j = 0; j < sortedIndexes.size(); j++)
			{
				if (curr < angles[sortedIndexes[j]])
					lowerThanIndex = j;
			}

			if (lowerThanIndex == -1)
				sortedIndexes.push_back(i);
			else
				sortedIndexes.insert(sortedIndexes.begin() + lowerThanIndex, i);
		}
	}

	std::vector<double*> finalList;
	finalList.resize(points.size());
	
	for (int i = 0; i < sortedIndexes.size(); i++)
		finalList[i] = points[sortedIndexes[i]];

	// swap middle points for 4p if intersections occur
	if (finalList.size() == 4 && finalList[0][0] > finalList[1][0])
	{
		std::vector<double*> tempList;
		tempList.resize(4);
		tempList[0] = finalList[0];
		tempList[1] = finalList[2];
		tempList[2] = finalList[1];
		tempList[3] = finalList[3];
		finalList = tempList;
	}

	return finalList;
}

double MinimalCrossSectionFinder::PolygonArea(std::vector<double*> sortedPoints)
{
	double area = 0;
	for (int i = 0; i < sortedPoints.size(); i++)
	{
		int j = i + 1 == sortedPoints.size() ? 0 : i + 1;
		area +=
			(sortedPoints[j][0] - sortedPoints[i][0]) *
			(sortedPoints[j][1] + sortedPoints[i][1]) / 2;
	}
	//std::cout << sortedPoints.size() << "P area: " << area << "\t Points: ";
	//for (int i = 0; i < sortedPoints.size(); i++)
	//	std::cout << "{" << sortedPoints[i][0] << "," << sortedPoints[i][1] << "} ";

	//std::cout << std::endl;
	return std::abs(area);
}

bool MinimalCrossSectionFinder::IsOnLine(IndexType point, IndexType anchorPoint, std::vector<double> normal, double* err)
{
	if (normal.empty()) return false;

	// line formula: x = x0 + at && y = y0 + bt && z = z0 + ct
	// where t = scale below
	auto scale = 0.0;
	if (normal[0] != 0)
		scale = (point[0] - anchorPoint[0]) / normal[0];
	else if (normal[1] != 0)
		scale = (point[1] - anchorPoint[1]) / normal[1];
	else if (normal[2] != 0)
		scale = (point[2] - anchorPoint[2]) / normal[2];
	else
		return false;

	auto x = scale * normal[0] + anchorPoint[0];
	auto y = scale * normal[1] + anchorPoint[1];
	auto z = scale * normal[2] + anchorPoint[2];

	auto xDiff = std::abs(x - point[0]);
	auto yDiff = std::abs(y - point[1]);
	auto zDiff = std::abs(z - point[2]);
	if (err != nullptr)
		*err = xDiff + yDiff + zDiff;
	return xDiff < 1 && yDiff < 1 && zDiff < 1;
}

RgbImage*
MinimalCrossSectionFinder::GetMinimalCrossSectionFrontImage()
{
	return GetCrossSectionImage(true);
}

RgbImage*
MinimalCrossSectionFinder::GetMinimalCrossSectionSideImage()
{
	return GetCrossSectionImage();
}

RgbImage* MinimalCrossSectionFinder::GetCrossSectionImage(bool front)
{
	auto volumeRegion = image->GetLargestPossibleRegion();
	auto volumeSize = volumeRegion.GetSize();
	auto volumeSpacing = image->GetSpacing();

	auto rgbImage = new RgbImage();
	auto newImage = RgbImageType::New();
	auto region = RgbImageType::RegionType();
	RgbImageType::IndexType start = { 0, 0 };
	RgbImageType::SizeType size;
	RgbImageType::SpacingType spacing = itk::Vector<double, 2U>();

	if (front)
	{
		size = { volumeSize[0], volumeSize[2] };
		spacing[0] = volumeSpacing[0];
		spacing[1] = volumeSpacing[2];

	}
	else
	{
		size = { volumeSize[1], volumeSize[2] };
		spacing[0] = volumeSpacing[1];
		spacing[1] = volumeSpacing[2];
	}

	region.SetSize(size);
	region.SetIndex(start);
	newImage->SetRegions(region);
	newImage->Allocate();
	newImage->SetSpacing(spacing);

	using IteratorType = itk::ImageRegionIterator<DicomSeriesType>;
	auto it = IteratorType(image, volumeRegion);
	it.GoToBegin();

	// select cross-sections
	while (!it.IsAtEnd())
	{
		if (it.Value() == SHRT_MAX)
		{
			RgbImageType::PixelType val;
			val.Set(255, 255, 255);
			auto in = it.GetIndex();
			DicomImageType::IndexType projIndex;
			if (front)
				projIndex = { in[0], in[2] };
			else
				projIndex = { in[1], in[2] };
			//if (IsOnPlane(in, calculatedSectionPoint, calculatedSectionNormal))
			//{
			//	// first section is red
			//	val.SetBlue(0); val.SetGreen(0);
			//	newImage->SetPixel(projIndex, val);
			//}
			//else if (IsOnPlane(in, calculatedSectionPoint2, calculatedSectionNormal2))
			//{
			//	// sectond is green
			//	val.SetBlue(0); val.SetRed(0);
			//	newImage->SetPixel(projIndex, val);
			//}
			//else if (IsOnPlane(in, calculatedSectionPoint3, calculatedSectionNormal3))
		    if (IsOnPlane(in, calculatedSectionPoint4, calculatedSectionNormal4)
				&& std::abs(in[1] - calculatedSectionPoint4[1]) < 30
				&& std::abs(in[2] - calculatedSectionPoint4[2]) < 30
				&& std::abs(in[0] - calculatedSectionPoint4[0]) < 40)
			{
				// third is violet
				val.SetGreen(0);
				newImage->SetPixel(projIndex, val);
			}
			else
				newImage->SetPixel(projIndex, val);
		}

		++it;
	}

	it.GoToBegin();
	// select lines

	std::vector<double> splitLineNormal;
	splitLineNormal.push_back(0);
	splitLineNormal.push_back((-1) / splitPlaneNormal[1]);
	splitLineNormal.push_back(splitPlaneNormal[2]);
	while (!it.IsAtEnd())
	{
		RgbImageType::PixelType val;
		val.Set(128, 128, 128);
		auto in = it.GetIndex();
		DicomImageType::IndexType projIndex;
		if (front)
			projIndex = { in[0], in[2] };
		else
			projIndex = { in[1], in[2] };
		if (/*IsOnLine(in, calculatedSectionPoint, calculatedSectionNormal)
			|| IsOnLine(in, calculatedSectionPoint2, calculatedSectionNormal2)
			|| */ IsOnLine(in, calculatedSectionPoint4, calculatedSectionNormal4)
			|| (!front && IsOnLine(in, splitPlanePoint, splitLineNormal)))
		{
			newImage->SetPixel(projIndex, val);
		}
		++it;
	}

	rgbImage->SetImage(newImage);
	return rgbImage;
}

Graph3D<MinimalCrossSectionFinder::IndexType>*
MinimalCrossSectionFinder::CreateMinimalPathsGraph()
{
	auto skeletonToGraph = new SkeletonToGraph();
	skeletonToGraph->SetInput(minimalPathsImage);
	auto graph = skeletonToGraph->GetGraph();

	delete skeletonToGraph;
	return graph;
}

MinimalCrossSectionFinder::~MinimalCrossSectionFinder()
{
	if (image != nullptr)
		image = nullptr;
	if (minimalPathsImage != nullptr)
		minimalPathsImage = nullptr;
}