#include "MinimalPathDijkstra.h"

void MinimalPathDijkstra::Initialize(unsigned int verticesCount,
	unsigned int start)
{
	paths = std::vector<unsigned int>(verticesCount, -1);
	costLookup = std::vector<unsigned int>(verticesCount, UINT32_MAX);
	doneVertices = std::vector<unsigned int>();
	doneVertices.reserve(verticesCount);
	undoneVertices = std::list<unsigned int>();
	for (unsigned int i = 0; i < verticesCount; i++)
		undoneVertices.push_back(i);

	costLookup[start] = 0;
}

std::vector<unsigned int>
MinimalPathDijkstra::GetMinimalPath(
	unsigned int start, unsigned int end,
	std::vector<unsigned int>** adjacencyMatrix, unsigned int verticesCount)
{
	Initialize(verticesCount, start);

	unsigned int doneCount = 0;
	unsigned int currentMinimal = start;
	while (doneCount < verticesCount)
	{
		currentMinimal = FindCurrentMinimal();
		//std::cout << "[Dijkstra] Current minimal: " << currentMinimal << "\t Cost: " << costLookup[currentMinimal] << std::endl;
		auto incidentVertices = GetIncidentVertices(currentMinimal, verticesCount, adjacencyMatrix);
		ResolveNeighborsMinimalPaths(incidentVertices, currentMinimal, adjacencyMatrix);

		SetAsDone(currentMinimal);
		doneCount++;
	}
	minimalCost = costLookup[end];
	return MinimalPathTo(end, verticesCount);
}

unsigned int
MinimalPathDijkstra::FindCurrentMinimal()
{
	if (undoneVertices.size() == 0) return -1;

	auto minIndex = -1;
	auto minCost = UINT32_MAX;
	for (auto it = undoneVertices.begin(); it != undoneVertices.end(); it++)
	{
		auto id = *it;
		auto cost = costLookup[id];
		if (cost < minCost)
		{
			minCost = cost;
			minIndex = id;
		}
	}

	return minIndex;
}

void MinimalPathDijkstra::ResolveNeighborsMinimalPaths(
	std::vector<unsigned int> neighbors, unsigned int current,
	std::vector<unsigned int>** adjMatrix)
{
	auto currentCost = costLookup[current];
	for (int i = 0; i < neighbors.size(); i++)
	{
		auto n = neighbors[i];
		auto edges = adjMatrix[current][n];
		auto minEdge = std::min_element(edges.begin(), edges.end());
		auto pathCost = currentCost + *minEdge;
		auto neighborCost = costLookup[n];

		auto isLowerCost = pathCost < neighborCost;
		if (isLowerCost)
		{
			costLookup[n] = pathCost;
			paths[n] = current;
		}

		//std::cout << "[Dijkstra] Neighbor: " << n
		//	<< "\t Check: " << neighborCost << " ?> " << pathCost << ": "
		//	<< isLowerCost << "\t " << (isLowerCost ? "Changing" : "") << std::endl;
	}
}

std::vector<unsigned int>
MinimalPathDijkstra::GetIncidentVertices(unsigned int current,
	unsigned int verticesCount,
	std::vector<unsigned int>** adjMatrix)
{
	auto incident = std::vector<unsigned int>();
	for (int i = 0; i < verticesCount; i++)
	{
		if (adjMatrix[current][i].size() > 0)
			incident.push_back(i);
	}

	return incident;
}

void MinimalPathDijkstra::SetAsDone(unsigned int id)
{
	auto it = std::find(undoneVertices.begin(), undoneVertices.end(), id);
	undoneVertices.remove(*it);
	doneVertices.push_back(*it);
}

std::vector<unsigned int>
MinimalPathDijkstra::MinimalPathTo(unsigned int id, unsigned int verticesCount)
{
	std::vector<unsigned int> minimalPath;
	minimalPath.reserve(verticesCount);

	unsigned int next = id;
	while (next != -1)
	{
		minimalPath.push_back(next);
		next = paths[next];
	}

	return minimalPath;
}

MinimalPathDijkstra::~MinimalPathDijkstra()
{
	std::cout << "MinimalPathDijkstra dctor... done." << std::endl;
}