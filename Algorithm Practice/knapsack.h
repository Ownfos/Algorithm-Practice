#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

struct Item
{
	int weight;
	int value;
};

void trim(std::string prefix, std::string postfix, std::string& data)
{
	data = data.substr(prefix.length(), data.length() - prefix.length() - postfix.length());
}

std::vector<Item> parse_file(std::string file_name, int amplifier)
{
	auto input = std::ifstream(file_name);

	// first line contains weight list
	// second line contains value list
	auto weight_data = std::string("");
	auto value_data = std::string("");
	std::getline(input, weight_data);
	std::getline(input, value_data);

	// trim prefix and postfix to get internal values delimited by ", "
	trim("weight [", "]", weight_data);
	trim("value [", "]", value_data);

	// convert input string to stream
	auto weight_stream = std::stringstream(weight_data);
	auto value_stream = std::stringstream(value_data);

	// buffer for removing ", " after each element
	auto comma = std::string("");

	auto result = std::vector<Item>();

	while (weight_stream)
	{
		double real_value;
		double real_weight;
		weight_stream >> real_weight >> comma;
		value_stream >> real_value >> comma;

		// convert real number data into approximated integer value.
		// more explanation can be found at main function's comment
		// about variable named "amplifier"
		result.push_back(
			{
				(int)std::round(real_weight * amplifier),
				(int)std::round(real_value * amplifier)
			}
		);
	}

	return result;
}

// solve 0-1 knapsack with raw recursion
int knapsack_dp_raw(const std::vector<Item>& items, int weight, int pos = 0)
{
	if (pos >= items.size())
		return 0;

	if (weight - items[pos].weight < 0)
	{
		return knapsack_dp_raw(items, weight, pos + 1);
	}
	else
	{
		return std::max(
			knapsack_dp_raw(items, weight, pos + 1),
			knapsack_dp_raw(items, weight - items[pos].weight, pos + 1) + items[pos].value
		);
	}
}

// solve 0-1 knapsack with memoization
int knapsack_dp(const std::vector<Item>& items, std::vector<std::vector<int>>& lookup, int weight, int pos = 0)
{
	if (pos >= items.size())
	{
		return 0;
	}

	if (lookup[pos][weight] == -1)
	{
		if (weight - items[pos].weight < 0)
		{
			lookup[pos][weight] = knapsack_dp(items, lookup, weight, pos + 1);
		}
		else
		{
			lookup[pos][weight] = std::max(
				knapsack_dp(items, lookup, weight, pos + 1),
				knapsack_dp(items, lookup, weight - items[pos].weight, pos + 1) + items[pos].value
			);
		}

	}

	return lookup[pos][weight];
}

int knapsack_dp_track_activation(const std::vector<Item>& items, std::vector<std::vector<int>>& lookup, std::vector<std::vector<std::tuple<int, int, bool>>>& activation, int weight, int pos = 0)
{
	if (pos >= items.size())
	{
		return 0;
	}

	if (lookup[pos][weight] == -1)
	{
		if (weight - items[pos].weight < 0)
		{
			activation[pos][weight] = { weight, pos + 1, false };
			lookup[pos][weight] = knapsack_dp_track_activation(items, lookup, activation, weight, pos + 1);
		}
		else
		{
			auto new_weight = weight - items[pos].weight;

			auto ignore = knapsack_dp_track_activation(items, lookup, activation, weight, pos + 1);
			auto include = knapsack_dp_track_activation(items, lookup, activation, new_weight, pos + 1) + items[pos].value;

			if (ignore > include)
			{
				activation[pos][weight] = { weight, pos + 1, false };
				lookup[pos][weight] = ignore;
			}
			else
			{
				activation[pos][weight] = { new_weight, pos + 1, true };
				lookup[pos][weight] = include;
			}
		}

	}

	return lookup[pos][weight];
}


// print whether each item is selected or not in binary form
void trace_activation(const std::vector<std::vector<std::tuple<int, int, bool>>>& activation, int weight, int pos = 0)
{
	if (pos >= activation.size())
		return;

	auto [next_weight, next_pos, included] = activation[pos][weight];
	std::cout << included ? "1" : "0";
	trace_activation(activation, next_weight, next_pos);
}

// checks if selection output does produce expected value and weight.
// "items" should be the data used on computing selection,
// so don't panic if error message shows up when
// validating 30.kp solution with 300.kp data.
void verify(const std::vector<Item>& items, std::string selection, int output_value, int max_weight)
{
	int value_sum = 0;
	int weight_sum = 0;
	for (int i = 0; i < selection.length(); ++i)
	{
		if (selection[i] == '1')
		{
			value_sum += items[i].value;
			weight_sum += items[i].weight;
		}
	}
	std::cout << "selection : " << selection << std::endl;
	std::cout << "total value : " << value_sum << std::endl;
	std::cout << "total weight : " << weight_sum << std::endl;

	if (output_value != value_sum || weight_sum > max_weight)
	{
		std::cout << "error : " << value_sum << " " << weight_sum << std::endl;
	}
}
/*
-----------OUTPUT RECORD-------------

30.kp max weight = 1)
	best value : 3.39815
	total weight : 0.90149
	000000000000001110000010101000

30.kp max weight = 3)
	best value : 6.03473
	total weight : 2.98259
	010100100100001110000010101000

300.kp max weight = 3)
	best value : 25.12116
	total weight : 2.99783
	000000000000001010000000001000000010000000000000000001010000010010000100000001010010000000000001010100000000000000000000000000100000000000000000001000000010000100000010000101000001000000000000100000100000110000000010000000000000000001000000010000010000000000000010000010000000000000010000000100010000
*/

#include <chrono>
using namespace std::chrono;

//148700
//22851800

int main()
{

	// since raw weight and values are small real numbers,
	// multiplying big number and rounding it gives good integer approximation.
	// amplifier of 10^n can be thought as n-digit precision.
	constexpr auto amplifier = 100000;
	auto max_weight = 3 * amplifier;

	// read and approximate data to integer values
	auto items = parse_file("30.kp", amplifier);

	// verify calculated outputs
	//std::cout << "verifing output 1" << std::endl;
	//verify(items, "000000000000001110000010101000", 339815, 1 * amplifier);
	//std::cout << "verifing output 2" << std::endl;
	//verify(items, "010100100100001110000010101000", 603473, 3 * amplifier);
	//std::cout << "verifing output 3" << std::endl;
	//verify(items, "000000000000001010000000001000000010000000000000000001010000010010000100000001010010000000000001010100000000000000000000000000100000000000000000001000000010000100000010000101000001000000000000100000100000110000000010000000000000000001000000010000010000000000000010000010000000000000010000000100010000", 2512116, 3 * amplifier);

	// create lookup table used on memoization
	std::vector<std::vector<int>> lookup(items.size());
	for (auto& weight_row : lookup)
		weight_row.resize(max_weight + 1, -1);

	// create stack trace table for tracking which items are selected
	std::vector<std::vector<std::tuple<int, int, bool>>> activation(items.size());
	for (auto& weight_row : activation)
		weight_row.resize(max_weight + 1);

	std::cout << "loading done" << std::endl;

	auto start = high_resolution_clock::now();
	// find max value using memoization with activation tracking
	auto solution_trace = knapsack_dp_track_activation(items, lookup, activation, max_weight);
	auto end = high_resolution_clock::now();
	auto duration = (end - start).count();
	std::cout << "took " << duration << " time" << std::endl;
	std::cout << "memoization with tracing : " << solution_trace << " (" << (double)solution_trace / amplifier << ")" << std::endl;

	std::cout << "item selection : ";
	trace_activation(activation, max_weight);
	std::cout << std::endl;

	// find max value using memoization
	auto solution = knapsack_dp(items, lookup, max_weight);
	std::cout << "memoization : " << solution << " (" << (double)solution / amplifier << ")" << std::endl;

	// find max value without using memoization
	auto solution_raw = knapsack_dp_raw(items, max_weight);
	std::cout << "raw : " << solution_raw << " (" << (double)solution_raw / amplifier << ")" << std::endl;
}