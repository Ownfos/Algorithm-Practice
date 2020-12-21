#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>

template<typename T>
using MinHeap = std::priority_queue<T, std::vector<T>, std::greater<T>>;

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
	for (auto& e : v)
		os << e << " ";
	return os;
}

struct Edge
{
	int src;
	int dest;
	double weight;

	bool operator>(const Edge& other) const
	{
		return weight > other.weight;
	}
};

struct City
{
	int id;
	double x;
	double y;

	double distance(const City& other) const
	{
		auto dx = x - other.x;
		auto dy = y - other.y;
		return std::sqrt(dx * dx + dy * dy);
	}

	friend std::istream& operator>>(std::istream& is, City& city)
	{
		return is >> city.id >> city.x >> city.y;
	}
};

class DistanceTable
{
public:
	DistanceTable(const std::vector<City>& cities)
		: distance(cities.size())
	{
		for (auto& row : distance)
			row.resize(cities.size());

		for (const auto& src : cities)
			for (const auto& dest : cities)
				distance[src.id][dest.id] = src.distance(dest);
	}

	double operator()(int city1, int city2) const
	{
		return distance[city1][city2];
	}

private:
	std::vector<std::vector<double>> distance;
};

class AdjacencyList
{
public:
	AdjacencyList(size_t size)
		: adjacent_edges(size)
	{}

	AdjacencyList(const std::vector<City>& cities)
		: adjacent_edges(cities.size())
	{
		for (const auto& src : cities)
			for (const auto& dest : cities)
				if(src.id != dest.id)
					adjacent_edges[src.id].push_back({ src.id, dest.id, src.distance(dest) });
	}

	void add_edge(Edge edge)
	{
		adjacent_edges[edge.src].push_back(edge);
		adjacent_edges[edge.dest].push_back(edge);
	}

	void sort_by_dest()
	{
		for (auto& e : adjacent_edges)
			std::sort(e.begin(), e.end(), [](const auto& lhs, const auto& rhs) {
				return lhs.dest < rhs.dest;
			});
	}

	void sort_by_weight()
	{
		for (auto& e : adjacent_edges)
			std::sort(e.begin(), e.end(), [](const auto& lhs, const auto& rhs) {
				return lhs.weight < rhs.weight;
			});
	}

	size_t size() const
	{
		return adjacent_edges.size();
	}

	const std::vector<Edge>& operator[](int src) const
	{
		return adjacent_edges[src];
	}

private:
	std::vector<std::vector<Edge>> adjacent_edges;
};

class Graph
{
public:
	Graph(const AdjacencyList& adjacency_list)
		: adjacency_list(adjacency_list)
	{
		this->adjacency_list.sort_by_dest();
	}

	Graph mst()
	{
		auto connection = AdjacencyList(adjacency_list.size());
		auto candidate = MinHeap<Edge>();
		auto visited = std::vector<bool>(connection.size(), false);
		auto start = 0;

		for (auto e : adjacency_list[start])
			candidate.push(e);

		visited[start] = true;

		while (!candidate.empty())
		{
			auto [src, dest, weight] = candidate.top();
			candidate.pop();

			if (!visited[dest])
			{
				visited[dest] = true;

				connection.add_edge({ src, dest, weight });

				for (auto e : adjacency_list[dest])
					candidate.push(e);
			}
		}

		return { connection };
	}

	std::vector<int> preorder_traversal()
	{
		auto result = std::vector<int>();
		auto visited = std::vector<bool>(adjacency_list.size(), false);
		auto stack = std::stack<int>();
		stack.push(0);

		while (!stack.empty())
		{
			auto next = stack.top();
			stack.pop();

			if (!visited[next])
			{
				visited[next] = true;

				result.push_back(next);

				// push the adjacent vertices to the stack in reverse order,
				// so that vertex with lower index is visited first.
				// note that adjacency list is sorted in the constructor.
				// ex) push order : 3,2,1 => pop order : 1,2,3
				auto adj = adjacency_list[next];
				for (auto it = adj.rbegin(); it != adj.rend(); ++it)
					stack.push(it->dest);
			}
		}

		return result;
	}

private:
	AdjacencyList adjacency_list;
};

class Path
{
public:
	Path() = default;

	Path(size_t length)
		: visited(length, false)
	{}

	Path(const std::vector<int>& complete_path)
		: path(complete_path), visited(complete_path.size(), true)
	{}

	void push(int city)
	{
		visited[city] = true;
		path.push_back(city);
	}

	void pop()
	{
		visited[path.back()] = false;
		path.pop_back();
	}

	int back() const
	{
		return path.back();
	}

	int length() const
	{
		return path.size();
	}

	bool is_visited(int city) const
	{
		return visited[city];
	}

	double full_cost(const DistanceTable& distance) const
	{
		double cost = 0.0;
		for (int i = 0; i < path.size(); ++i)
			cost += distance(path[i], path[(i + 1) % path.size()]);
		return cost;
	}

	bool operator<(const Path& path) const
	{
		return true;
	}

	// try all possible case of swapping two cities in path
	// until no cost reduction happens.
	void evolve(const DistanceTable& distance)
	{
		auto current_cost = full_cost(distance);
		while (true)
		{
			auto updated = false;
			for (int i = 1; i < path.size(); ++i)
			{
				for (int j = i + 1; j < path.size(); ++j)
				{
					std::swap(path[i], path[j]);
					auto new_cost = full_cost(distance);
					if (new_cost < current_cost)
					{
						updated = true;
						current_cost = new_cost;
					}
					else
					{
						std::swap(path[i], path[j]);
					}
				}
			}
			if (updated)
			{
				std::cout << "## improved cost : " << current_cost << std::endl;
				std::cout << "## path : " << path << std::endl;
			}
			else
			{
				break;
			}
		}
	}

	double lower_bound(const DistanceTable& distance, const AdjacencyList& adjacency_list) const
	{
		double lb = 0.0;

		// edge weights between visited cities
		for(int i=0;i<path.size()-1;++i)
			lb += distance(path[i], path[i + 1]);

		// minimum edge cost for returning to first city
		for (auto [src, dest, weight] : adjacency_list[path.front()])
		{
			if (!visited[dest])
			{
				lb += weight / 2.0;
				break;
			}
		}

		// minimum edge cost for departing from last city
		for (auto [src, dest, weight] : adjacency_list[path.back()])
		{
			if (!visited[dest])
			{
				lb += weight / 2.0;
				break;
			}
		}

		// minimum adjacent edge cost for unvisited cities
		for (int city = 0; city < visited.size(); ++city)
		{
			if (!visited[city])
			{
				// average weigth of two minimum cost edge starting from unvisited city.
				int count = 2;
				for (auto [src, dest, weight] : adjacency_list[city])
				{
					if (dest == path.front() || dest == path.back() || !visited[dest])
					{
						lb += weight / 2.0;
						if (--count == 0)
							break;
					}
				}
			}
		}

		return lb;
	}

	friend std::ostream& operator<<(std::ostream& os, const Path& path)
	{
		return os << path.path;
	}

private:
	std::vector<int> path;
	std::vector<bool> visited;
};

void branch_bound(
	Path& temp_path,
	Path& best_path,
	const DistanceTable& distance_table,
	const AdjacencyList& adjacency_list
)
{
	// debug info
	static int call_count = 0;
	static int prune_count = 0;
	static unsigned long long removed = 0;
	static int branch_length = 0;

	// show progress
	if (call_count >= 10000000)
	{
		std::cout << std::endl << std::endl;
		std::cout << "prune ratio : " << (double)prune_count / call_count * 100 << "%" << std::endl;
		std::cout << "avg branch length : " << (double)branch_length / call_count << std::endl;
		std::cout << "best cost : " << best_path.full_cost(distance_table) << std::endl;
		std::cout << "best path : " << best_path << std::endl;
		std::cout << "temp path : " << temp_path << std::endl << std::endl;
		call_count = 0;
		prune_count = 0;
		branch_length = 0;
	}

	auto best_cost = best_path.full_cost(distance_table);
	if (temp_path.length() == adjacency_list.size() - 1)
	{
		auto cost = temp_path.lower_bound(distance_table, adjacency_list);

		// add last unvisited city to path
		for (int last_city = 0; last_city < adjacency_list.size(); ++last_city)
		{
			if (!temp_path.is_visited(last_city))
			{
				temp_path.push(last_city);
				break;
			}
		}

		// update best_cost
		if (best_cost > cost)
		{
			best_path = temp_path;
			std::cout << std::endl;
			std::cout << "# new path found : " << cost << std::endl;
			std::cout << "# path : " << best_path << std::endl;
			best_path.evolve(distance_table);
		}

		// remove the last unvisited city.
		// the return position of current function call expectes temp_path to maintain its length
		// as adjacency_list.size() - 1, which is total city - 1.
		temp_path.pop();
	}
	else
	{
		// sort unvisited cities in ascending order of lower-bound
		auto branch_order = std::vector<std::pair<double, int>>();
		for (auto next_city = 0; next_city < adjacency_list.size(); ++next_city)
		{
			if (!temp_path.is_visited(next_city))
			{
				temp_path.push(next_city);
				auto lower_bound = temp_path.lower_bound(distance_table, adjacency_list);
				temp_path.pop();

				branch_order.emplace_back(lower_bound, next_city);
			}
		}
		std::sort(branch_order.begin(), branch_order.end());

		// branch or prune
		for (auto [lower_bound, next_city] : branch_order)
		{
			++call_count;
			branch_length += temp_path.length();

			temp_path.push(next_city);

			if (lower_bound < best_cost)
			{
				branch_bound(temp_path, best_path, distance_table, adjacency_list);
			}
			else
			{
				if(++prune_count % 1000000 == 0)
					std::cout << temp_path << std::endl;
			}

			temp_path.pop();
		}

	}
}

int main()
{
	std::ios::sync_with_stdio(false);

	auto file = std::ifstream("100.tsp");
	auto cities = std::vector<City>(100);
	for (auto& city : cities)
		file >> city;

	auto distance_table = DistanceTable(cities);
	auto adjacency_list = AdjacencyList(cities);
	adjacency_list.sort_by_weight();

	auto graph = Graph(adjacency_list);
	auto mst = graph.mst();
	auto two_approx = Path(mst.preorder_traversal());

	std::cout << two_approx << std::endl;
	std::cout << two_approx.full_cost(distance_table) << std::endl;

	auto best_path = two_approx;
	auto temp_path = Path(cities.size());
	temp_path.push(0);
	branch_bound(temp_path, best_path, distance_table, adjacency_list);
}