#pragma once
#include "common.h"
#include <algorithm>
#include <list>

/*

Minimal path extraction in undirectional graph using Dijkstra's algorithm.
Graph may containt doubled edges, in which case the minimal edge
will be taken into consideration.

*/

class MinimalPathDijkstra
{
public:
	MinimalPathDijkstra() { };
	~MinimalPathDijkstra();

	std::vector<unsigned int> GetMinimalPath(
		unsigned int start, unsigned int end,
		std::vector<unsigned int>** adjacencyMatrix, unsigned int verticesCount);
	unsigned int GetMinimalCost() { return minimalCost; }

private:

	void Initialize(unsigned int verticesCount,
		unsigned int start);
	unsigned int FindCurrentMinimal();
	void ResolveNeighborsMinimalPaths(std::vector<unsigned int> neighbors, unsigned int current,
		std::vector<unsigned int>** adjMatrix);
	std::vector<unsigned int> GetIncidentVertices(unsigned int current,
		unsigned int verticesCount,
		std::vector<unsigned int>** adjMatrix);
	void SetAsDone(unsigned int id);
	std::vector<unsigned int> MinimalPathTo(unsigned int id, unsigned int verticesCount);

	std::vector<unsigned int> paths;
	std::vector<unsigned int> doneVertices;
	std::list<unsigned int> undoneVertices;

	std::vector<unsigned int> costLookup;
	unsigned int minimalCost = 0;
};