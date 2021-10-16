#pragma once
#include "common.h"
#include <algorithm>
#include <queue>

/*

Minimal path extraction in undirectional graph using Dijkstra's algorithm.
Graph may containt doubled edges, in which case the minimal edge
will be taken into consideration.

*/

class AllPathsBFS
{
public:
	AllPathsBFS() { };
	~AllPathsBFS() { };

	std::vector<std::vector<unsigned int>> GetAllPaths(
		unsigned int start, unsigned int end,
		std::vector<unsigned int>** adjacencyMatrix, unsigned int verticesCount);

private:
	void Initialize(unsigned int verticesCount, unsigned int start);
	void ClearVisited();
	bool IsVisited(unsigned int id);
	void SetAsVisited(unsigned int id);

	std::queue<std::vector<unsigned int>> queue;
	std::vector<unsigned int> currentPath;
	std::vector<std::vector<unsigned int>> allPaths;

	std::vector<bool> visitedLookup;
};