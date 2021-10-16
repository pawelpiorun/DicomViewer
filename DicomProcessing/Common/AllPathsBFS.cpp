#include "AllPathsBFS.h"

std::vector<std::vector<unsigned int>> AllPathsBFS::GetAllPaths(
	unsigned int start, unsigned int end,
	std::vector<unsigned int>** adjMatrix, unsigned int verticesCount)
{
	Initialize(verticesCount, start);

	while (!queue.empty())
	{
		currentPath = queue.front();
		queue.pop();
		auto last = currentPath[currentPath.size() - 1];
		SetAsVisited(last);

		// if last vertex is the desired destination
		// then add to all paths
		if (last == end)
			allPaths.push_back(currentPath);
		
		// traverse to all the nodes connected to
		// current vertex and push new path to queue
		for (unsigned int i = 0; i < verticesCount; i++)
		{
			if (adjMatrix[last][i].size() == 0) continue;
			if (IsVisited(i)) continue;

			std::vector<unsigned int> newPath(currentPath);
			newPath.push_back(i);
			queue.push(newPath);
		}
	}

	return allPaths;
}

void AllPathsBFS::Initialize(unsigned int verticesCount, unsigned int start)
{
	visitedLookup = std::vector<bool>();
	visitedLookup.resize(verticesCount);
	ClearVisited();

	currentPath.clear();
	currentPath.push_back(start);

	while (!queue.empty())
		queue.pop();
	queue.push(currentPath);
}

void AllPathsBFS::ClearVisited()
{
	std::fill(visitedLookup.begin(), visitedLookup.end(), false);
}

bool AllPathsBFS::IsVisited(unsigned int id)
{
	return visitedLookup[id];
}

void AllPathsBFS::SetAsVisited(unsigned int id)
{
	visitedLookup[id] = true;
}
