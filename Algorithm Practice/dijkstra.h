#pragma once
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <thread>
#include <future>

constexpr int INFINITE = 999999999;

struct Edge
{
	int to;
	int weight;
};

std::vector<int> dijkstra(int num_vertices, int source, const std::vector<std::vector<Edge>>& edges)
{
	std::vector<int> path_weight(num_vertices, INFINITE);
	std::vector<int> visited(num_vertices, false);
	std::priority_queue<
		std::pair<int, int>,
		std::vector<std::pair<int, int>>,
		std::greater<std::pair<int, int>>
	> queue;
	queue.push({ 0, source });
	path_weight[source] = 0;

	while (!queue.empty())
	{
		auto [cost, next] = queue.top();
		queue.pop();

		if (!visited[next]) {
			visited[next] = true;

			for (const auto& [to, weight] : edges[next])
			{
				auto new_weight = path_weight[next] + weight;
				if (path_weight[to] > new_weight)
				{
					path_weight[to] = new_weight;
					queue.push({ new_weight, to });
				}
			}
		}
	}

	return path_weight;
}

int main()
{
	std::iostream::sync_with_stdio(false);

	// Input file configuration
	//auto num_vertices = 1000000;
	//auto input_file = std::ifstream("1000000.graph");
	//auto num_vertices = 32000;
	//auto input_file = std::ifstream("32000.graph");
	auto num_vertices = 16000;
	auto input_file = std::ifstream("16000.graph");

	// Multithreading configuration.
	// num_groups : number of vertex group to process at one time.
	// num_threads : number of threads created for each group.
	// For example, (num_group = 2, num_vertices = 1000, num_threads = 10) means
	// vertex 0 ~ 999 is divided into two groups 0~499 and 500~999. Each group's vertices
	// is distibuted to four threads evenly to find the local longest shortest path solution whith that vertex as source node
	// (e.g. lsp of vertex 0~49 from thread 1, lsp of vertex 50~99 from thread 2, ...)
	auto num_groups = 1000;
	auto num_threads = 8;

	// Contain all directed edge information in form edges[from] = {from->dest1, from->dest2, ...}.
	// In other words, ith element of edges correspond to list of edges outgoing from ith vertex.
	auto edges = std::vector<std::vector<Edge>>(num_vertices);
	while (input_file)
	{
		int from, to, weight;
		input_file >> from >> to >> weight;
		edges[from].push_back({ to, weight });
	}

	return dijkstra(num_vertices, 12656, edges)[4569];


	// Temporary storage to save the lastest longest shortest path solution.
	// After each thread returns their local solutions, they are compared with this
	// global solution and update it if the local solution was better (i.e. found longer shortest path)
	auto max_src = 0;
	auto max_dest = 0;
	auto max_weight = 0;
	
/*

16000.graph result : 12657 -> 4569 : 107
32000.graph result : 28850 -> 12334 : 131
1000000.graph intermediate result :
405591 125606 181
460789 101238 176
516243 119126 177
522562 170395 182
533221 170395 186
648038 166859 188
668343 173976 189
773670 173976 190
842487 252404 197
*/


	// First two for-loop calculates distribution of vertices on each thread.
	// The threads will perform dijkstra's algorithm with source as vertex t_start ~ t_end.
	auto source_per_group = num_vertices / num_groups;
	for (auto group = 0; group < num_groups; ++group)
	{
		std::cout << "group : " << group << std::endl;
		auto start = source_per_group * group;
		auto end = group == num_groups - 1 ? num_vertices : start + source_per_group;

		auto source_per_thread = (end - start) / num_threads;
		auto results = std::vector<std::future<std::tuple<int, int, int>>>();
		for (auto t = 0; t < num_threads; ++t)
		{
			auto t_start = start + source_per_thread * t;
			auto t_end = t == num_threads - 1 ? end : t_start + source_per_thread;

			// Start a new thread and save the std::future instance to get result later.
			results.emplace_back(std::async(std::launch::async, [&, t_start, t_end] {
				auto max_src = 0;
				auto max_dest = 0;
				auto max_weight = 0;
				for (auto source = t_start; source < t_end; ++source) {
					auto paths = dijkstra(num_vertices, source, edges);

					// Update if any shortest path with source vertex "source" is longer than local optima (but not disconnected, i.e. INFINITE)
					for (int dest = 0; dest < num_vertices; ++dest) {
						if (paths[dest] > max_weight && paths[dest] != INFINITE)
						{
							max_src = source;
							max_dest = dest;
							max_weight = paths[dest];
						}
					}
				}

				std::cout << "thread end\n";
				return std::tuple{ max_src, max_dest, max_weight };
				}));
		}

		// After all threads finish their task, get the local solutions to
		// update the global solution.
		for (auto& result : results)
		{
			auto [src, dest, weight] = result.get();
			if (max_weight < weight && weight != INFINITE)
			{
				max_src = src;
				max_dest = dest;
				max_weight = weight;
			}
		}

		std::cout << "                                 " << max_src << " " << max_dest << " " << max_weight << std::endl;
	}

	system("pause");

	//for (auto source = 0; source < num_vertices; ++source)
	//{
	//	std::cout << "pass : " << source << std::endl;
	//	auto start = std::chrono::steady_clock::now();

	//	auto path = dijkstra(num_vertices, 12657, edges);
	//	auto end = std::chrono::steady_clock::now();
	//	//std::cout << "duration : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()/1000.0 << std::endl;

	//	for (auto& p : path)
	//		output_file << p << " ";
	//	output_file << std::endl;
	//}
}