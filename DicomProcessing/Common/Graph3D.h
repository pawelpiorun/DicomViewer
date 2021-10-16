#pragma once
#include <vector>
#include <map>

template <typename T>
struct Vertex
{
	Vertex()
	{
		id = -1;
	}

	Vertex(unsigned int _d, T _in, unsigned int _id)
	{
		degree = _d;
		index = _in;
		id = _id;
	}
	unsigned int id;
	unsigned int degree;
	T index;
};

template <typename T>
class Graph3D
{
public:
	Graph3D(std::vector<unsigned int>** adjacencyMatrix, std::vector<Vertex<T>> vertices,
		std::vector<T> indexLookup, std::map<unsigned int, std::map<unsigned int, std::vector<T>>> edgesLookup)
	{
		this->adjacencyMatrix = adjacencyMatrix;
		this->vertices = vertices;
		this->indexLookup = indexLookup;
		this->edgesLookup = edgesLookup;
		this->verticesCount = vertices.size();
		for (int i = 0; i < vertices.size(); i++)
			if (vertices[i].id > maxId) maxId = vertices[i].id;
	}
	~Graph3D()
	{
		std::cout << "Graph dctor... ";
		std::cout << "Adjacency matrix: " << &(adjacencyMatrix[0]) << " ";
		
		for (unsigned int i = 0; i < verticesCount; i++)
			delete[] adjacencyMatrix[i];
		delete[] adjacencyMatrix;
		adjacencyMatrix = nullptr;
		std::cout << "done." << std::endl;
	}

	std::vector<unsigned int> GetAdjacentVertices(unsigned int id)
	{
		auto map = edgesLookup[id];
		std::vector<unsigned int> keys;
		for (auto it = map.begin(); it != map.end(); ++it)
			keys.push_back(it->first);
		return keys;
	}
	std::vector<unsigned int>** GetAdjacencyMatrix() { return adjacencyMatrix; }
	std::vector<Vertex<T>> GetVertices() { return vertices;	}
	unsigned int GetVerticesCount() { return verticesCount; }
	T GetVertexIndex(unsigned int id) { return indexLookup[id]; }
	std::vector<T> GetEdgeIndexes(unsigned int first, unsigned int second) { return edgesLookup[first][second]; }
	std::map<unsigned int, std::map<unsigned int, std::vector<T>>> GetEdgesLookup() { return edgesLookup; }
	void RemoveVertex(unsigned int id)
	{
		std::cout << "[Graph] Removing vertex: " << id << " ... ";
		auto it = vertices.begin();
		for (; it != vertices.end(); it++)
			if (it->id == id) break;
		vertices.erase(it);

		std::cout << "Erased from vertices ... ";

		indexLookup.erase(indexLookup.begin() + id);
		edgesLookup.erase(id);

		std::cout << "Erased from lookups ... ";

		for (unsigned int i = 0; i < maxId; i++)
		{
			adjacencyMatrix[id][i] = std::vector<unsigned int>();
			adjacencyMatrix[i][id] = std::vector<unsigned int>();
			edgesLookup[i].erase(id);
		}
		std::cout << "Erased from adjacencyMatrix ...";
		std::cout << "done." << std::endl;
	};

private:
	unsigned int maxId;
	unsigned int verticesCount;
	std::map<unsigned int, std::map<unsigned int, std::vector<T>>> edgesLookup;
	std::vector<T> indexLookup;
	std::vector<unsigned int>** adjacencyMatrix = nullptr;
	std::vector<Vertex<T>> vertices;
};