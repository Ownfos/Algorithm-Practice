# Algorithm-Practice
This repository stores solution for several algorithm problems written in modern c++.
Each header file contains its driving main function,
so uncommenting respective header in main.cpp is all you need to run the program.

## knapsack
Given list of each item's weight and value,
find the subset of items which maximizes value within allowed weight budget.
weight and value are real numbers, given as txt file with following format:
"weight = [1.0, 2.0, ...]\nvalue = [1.0, 2.0, ...]"

The solution uses dynamic programming.
In order to use memoization technique, all input data are appriximated to large integer.

## tsp
Given list of cities and their x and y coordinate, find the tour path that minimizes total travel distance.
Input data are given as txt file with each line containing data about a city in following format:
"[city index] [x coordinate] [y coordinate]"
The coordinates are given as real numbers.

The solution uses branch and bound with 2-approximation (minimum spanning tree) as initial optimal value.

## dijkstra
Given an adjacency list, find the longest path among all-pair shortest paths.
In other words, you have to find two vertices such that the shortest path between them is
longer than the shortest path of any other two vertices.
Input data for each edge are given in following format: "[vertex1] [vertex2] [edge weight]"

The solution performs dijkstra's algorithm for each vertex as starting point.
Since each function call for SSSP(single source shortest path) is independent from each other,
I divided vertices into groups and applied mutithreading for each group to speed up the process.
