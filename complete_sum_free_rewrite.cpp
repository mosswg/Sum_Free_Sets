//
// Created by moss on 10/18/22.
//
#include "complete_sum_free_sets_consts.h"
#include <fstream>
#include <cstdint>
#include <bitset>
#include <cmath>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

typedef __uint128_t sum_free_set_t;
typedef uint32_t set_bound_t;

#define SUM_FREE_SETS_FILE_OUTPUT
#define TIME_SUM_FREE_SETS_GENERATION
// #define SUM_FREE_SETS_GRAPH_OUTPUT
// #define SUM_FREE_SETS_LOG_GRAPH

// https://en.wikipedia.org/wiki/Circular_shift
sum_free_set_t rotl(sum_free_set_t value, set_bound_t count, set_bound_t n) {
	return value << count | value >> (n - count);
}

void write_set_to_file(sum_free_set_t set, set_bound_t n, std::ofstream& outfile) {
	outfile << "{";
	for (set_bound_t i = 0; i < n; i++) {
		if ((set >> i) & 0b1) {
			outfile << i;

			if ((set >> (i + 1))) {
				outfile << ", ";
			}
		}
	}
	outfile << "},\n";
}

void write_set_tree_to_file(sum_free_set_t set, set_bound_t n, std::ofstream& outfile) {
	set_bound_t last_value = -1;
	set_bound_t size = 0;
	for (set_bound_t i = 1; i < n; i++) {
		if ((set >> i) & 0b1) {
			size++;
			outfile << "\tnode_" << i << "_" << size << " [label=\"" << i << "\"];\n";
			if (last_value != -1) {
				outfile << "\tnode_" << last_value << "_" << size - 1 << "->";
				outfile << "node_" << i << "_" << size << ";\n";
			}
			last_value = i;
		}
	}
	outfile << "\n\n";
}

set_bound_t get_set_bits(sum_free_set_t value) {
	set_bound_t count = 0;
	while(value){
		value &= value-1;
		count++;
	}
	return count;
}

void print_set(sum_free_set_t set, set_bound_t n) {
	printf("{");
	for (set_bound_t i = 0; i < n; i++) {
		if ((set >> i) & 0b1) {
			printf("%d", i);

			if ((set >> (i + 1))) {
				printf(", ");
			}
		}
	}
	printf("},\n");
}

// Using 256 because it is twice the max supported n (128) value for now; 2500 is arbitrary
sum_free_set_t complete_sum_free_sets_for_thread[256][2500];
set_bound_t current_complete_sum_free_set_for_thread[256];
set_bound_t total_number_of_complete_sum_free_sets;
set_bound_t minimum_found_set_length;
sum_free_set_t mask;
set_bound_t max_first_value;

void initialize_for_n(set_bound_t n) {
	minimum_found_set_length = n;
	max_first_value = ((n + 1) / 3);
	mask = ((((sum_free_set_t)1) << (n)) - 1);

	for(int i = 0; i < 128; i++) {
		current_complete_sum_free_set_for_thread[i] = 0;
	}
}

void print_sets(set_bound_t n) {
	sum_free_set_t set;

	for (set_bound_t starting_value = 1; starting_value <= max_first_value; starting_value++) {
		for (set_bound_t i = 0; (set = complete_sum_free_sets_for_thread[starting_value][i]); i++) {
			print_set(set, n);
		}
	}
	std::cout << '\n';
}

void write_sets_to_file(set_bound_t n, double compute_time = 0) {
	sum_free_set_t set;
	std::ofstream outfile;
	if (n < 10) {
		outfile.open("output_files/complete_sum_free_sets_0" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}
	else {
		outfile.open("output_files/complete_sum_free_sets_" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}


	std::ofstream graph_outfile;
	if (n < 10) {
		graph_outfile.open("tree_files/complete_sum_free/0" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}
	else {
		graph_outfile.open("tree_files/complete_sum_free/" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}

	graph_outfile << "digraph BST {\n";


	for (set_bound_t starting_value = 1; starting_value <= max_first_value; starting_value++) {
		for (set_bound_t i = 0; (set = complete_sum_free_sets_for_thread[starting_value][i]); i++) {
			write_set_to_file(set, n, outfile);
			write_set_tree_to_file(set, n, graph_outfile);
		}
	}
	if (compute_time != 0) {
		outfile << "Time: " << compute_time << " ms\n";
	}
	outfile.close();

	graph_outfile << "}";
	graph_outfile.close();

	std::cout << '\n';
}

sum_free_set_t generate_sums(sum_free_set_t set, set_bound_t n) {
	sum_free_set_t sums = 0;

	for (set_bound_t i = 1; set >> i; i++) {
		if ((set >> i) & 0b1) {
			sums |= rotl(set, i, n);
		}
	}

	return sums;
}

void mirror_first_half_of_set(sum_free_set_t set, set_bound_t n) {
	for (set_bound_t i = 0; i <= (n / 2); i++) {
		set |= ((set >> i) & 0b1) << (n - i);
	}
}

bool is_complete_sum_free(const sum_free_set_t set, set_bound_t n) {
	sum_free_set_t sums = 0;

	for (sum_free_set_t i = 0; i <= n; i++) {
		// Add i to each value of the set and or with the sums if the set contains i
		if ((set >> i) & 0b1) {
			sums |= rotl(set, i, n);

			// Check if not sum-free
			if ((sums & (set)) & mask) {
				return false;
			}
		}
	}

	// Check if the sums are equal to the complementary set
	return ((sums & mask) == ((~set) & mask));
}

bool check_for_asymmetric_sets(set_bound_t n) {
	sum_free_set_t mask = (((sum_free_set_t)1) << ((n + 1) / 2)) - 1;
	bool found = false;
	for (set_bound_t starting_value = 1; starting_value <= max_first_value; starting_value++) {
		for (set_bound_t i = 0; i < current_complete_sum_free_set_for_thread[starting_value]; i++) {
			sum_free_set_t current_set = complete_sum_free_sets_for_thread[starting_value][i];
			if (!current_set) {
				continue;
			}
			sum_free_set_t half_set = current_set & mask;
			mirror_first_half_of_set(half_set, n);
			if (half_set != current_set) {
				std::cout << "Found asymmetric set" << std::endl;
				print_set(current_set, n);
				found = true;
			}
		}
	}
	return found;
}

bool is_complete(const sum_free_set_t set, set_bound_t n) {
	sum_free_set_t sums = 0;

	for (set_bound_t i = 0; i <= n; i++) {
		// Add i to each value of the set and or with the sums if the set contains i
		if ((set >> i) & 0b1) {
			sums |= rotl(set, i, n);
		}
	}

	// Check if the sums are equal to the complementary set
	return ((sums & mask) == ((~set) & mask));
}

set_bound_t get_last_node_value(sum_free_set_t set) {
	set_bound_t i = 1;
	for (; set >> i; i++);
	return i - 1;
}

uint32_t generate_sub_nodes(sum_free_set_t set, sum_free_set_t sums, set_bound_t last_node_value, set_bound_t size, set_bound_t n, set_bound_t thread_index) {
	sum_free_set_t new_set;
	sum_free_set_t new_set_sums;
	sum_free_set_t new_value_to_add;
	set_bound_t number_of_new_sum_free_sets = 0;
	set_bound_t double_last_node_value = last_node_value * 2;


	for (set_bound_t i = last_node_value + 1; i < n; i++) {
		if (i == double_last_node_value) continue;
		new_value_to_add = ((sum_free_set_t)1) << i;

		// Ignore any values that add
		if (new_value_to_add & sums & mask) continue;

		new_set = set | new_value_to_add;

		new_set_sums = sums | rotl(new_set, i, n);

		if (!((new_set_sums & (new_set)) & mask)) {

			int num_generated = generate_sub_nodes(new_set, new_set_sums, i, size + 1, n, thread_index);
			if (num_generated == 0) {
				if (((new_set_sums & mask) == ((~new_set) & mask))) {
					if (size < minimum_found_set_length) {
						minimum_found_set_length = size;
					}
					complete_sum_free_sets_for_thread[thread_index][current_complete_sum_free_set_for_thread[thread_index]++] = new_set;
				}
			}
			number_of_new_sum_free_sets++;
		}
	}

	return number_of_new_sum_free_sets;
}

void generate_nodes_from_set(sum_free_set_t set, sum_free_set_t sums, set_bound_t last_node_value, set_bound_t size, set_bound_t n, set_bound_t thread_index) {
	uint32_t num_new_sets = generate_sub_nodes(set, sums, last_node_value, size, n, thread_index);

	if (num_new_sets == 0) {
		if (((sums & mask) == ((~set) & mask))) {
			if (size < minimum_found_set_length) {
				minimum_found_set_length = size;
			}
			complete_sum_free_sets_for_thread[thread_index][current_complete_sum_free_set_for_thread[thread_index]++] = set;
		}
	}
	// Output finished thread index
	std::cout << (thread_index <= max_first_value ? std::to_string(thread_index) : "1-" + std::to_string(thread_index - max_first_value)) << ' ' << std::flush;

	return;
}

void generate_nodes_from_starting_value(set_bound_t value, set_bound_t n) {

#ifdef SUM_FREE_SETS_GRAPH_OUPUT
	std::string file_path;
	if (n < 10) {
		file_path = "tree_files/sum_free/0" + std::to_string(n);
	}
	else {
		file_path = "tree_files/sum_free/" + std::to_string(n);
	}

	if (!std::filesystem::is_directory(file_path) || !std::filesystem::exists(file_path)) { // Check if src folder exists
		std::filesystem::create_directory(file_path); // create src folder
	}
	std::ofstream outfile(file_path + "/" + std::to_string(value) + "_tree.dot", std::ofstream::out | std::ofstream::trunc);
	outfile << "digraph BST {\n\tnode [fontname=\"Arial\"];\n";

	outfile << "\tnode_" << value << "_1 [label=\"" << value << "\"];\n";
#endif

	sum_free_set_t starting_value = ((sum_free_set_t) 0b1) << value;

	generate_nodes_from_set(starting_value, rotl(starting_value, value, n), value, 1, n, value);

#ifdef SUM_FREE_SETS_GRAPH_OUTPUT
	outfile << "}";
	outfile.close();
#endif
}



void generate_nodes_from_1_with_threads(set_bound_t n, std::vector<std::thread>& threads) {
	sum_free_set_t set = 0b010;
	sum_free_set_t sums = 0b100;
	set_bound_t size = 1;
	sum_free_set_t new_set;
	sum_free_set_t new_set_sums;
	sum_free_set_t new_value_to_add;


	for (set_bound_t i = 3; i < n; i++) {
		new_value_to_add = ((sum_free_set_t)1) << i;

		new_set = set | new_value_to_add;

		new_set_sums = sums | rotl(new_set, i, n);

		if (!((new_set_sums & (new_set)) & mask)) {
			std::cout << 1 << '-' << i << ' ';
			threads.emplace_back(generate_nodes_from_set, new_set, new_set_sums, i, size + 1, n, max_first_value + i);
		}
	}

	return;
}

	// This is somewhat arbitrary; We just need no complete sum free sets with length 2 as there is no check for that in generate_nodes_from_1_with_threads
	// This only saves time if n is even
const set_bound_t min_n_for_1_threading = 25;
void generate_complete_sum_free_sets(set_bound_t n) {
	std::vector<std::thread> threads;
	set_bound_t simple_starting_value = 1;
	std::cout << "start: ";
	if (n > min_n_for_1_threading && n % 2 == 0) {
		generate_nodes_from_1_with_threads(n, threads);
		simple_starting_value = 2;
	}
	for (set_bound_t i = simple_starting_value; i <= max_first_value; i++) {
		std::cout << i << ' ' << std::flush;
		threads.emplace_back(generate_nodes_from_starting_value, i, n);
	}
	std::cout << "\ndone: " << std::flush;

	/// Ensure that threads are done before exiting
	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}

	total_number_of_complete_sum_free_sets = 0;
	for (set_bound_t i = 1; i <= max_first_value + n; i++) {
		total_number_of_complete_sum_free_sets += current_complete_sum_free_set_for_thread[i];
	}


	/// Combine 1 values if necessary
	if (simple_starting_value > 1) {
		for (set_bound_t i = max_first_value + 1; i < max_first_value + n; i++) {
			for (set_bound_t j = 0; j < current_complete_sum_free_set_for_thread[i]; j++) {
				complete_sum_free_sets_for_thread[1][current_complete_sum_free_set_for_thread[1]++] = complete_sum_free_sets_for_thread[i][j];
			}
		}
	}

	std::cout << std::endl;
}


double log_10_2 = log10(2);
set_bound_t print_all_complete_sum_free_sets_new(set_bound_t n) {
	initialize_for_n(n);
	std::cout << "Generating Sum Free Sets" << std::endl;

#ifdef TIME_SUM_FREE_SETS_GENERATION
    auto t_start = std::chrono::high_resolution_clock::now();
#endif

	generate_complete_sum_free_sets(n);

#ifdef TIME_SUM_FREE_SETS_GENERATION
    auto t_end = std::chrono::high_resolution_clock::now();
#endif


	print_sets(n);

#ifdef SUM_FREE_SETS_LOG_GRAPH
	std::ofstream("output_files/complete_sum_free_sets_log_graph_points", std::ios_base::app) << "(" << n << "," << (log10(current_complete_sum_free_set) / log_10_2) << ")\n";
#endif

#ifdef SUM_FREE_SETS_FILE_OUTPUT
	#ifdef TIME_SUM_FREE_SETS_GENERATION
	write_sets_to_file(n, std::chrono::duration<double, std::milli>(t_end-t_start).count());
	#else
	write_sets_to_file(n);
	#endif
#endif

	std::cout << "Found: " << total_number_of_complete_sum_free_sets << std::endl;
	std::cout << "Minimum Set Length: " << (minimum_found_set_length == n ? 0 : minimum_found_set_length) << std::endl;

	return total_number_of_complete_sum_free_sets;
}

set_bound_t print_symmetric_complete_sum_free_sets(set_bound_t n) {
	initialize_for_n(n);
	set_bound_t num_found = 0;
	sum_free_set_t limit = 1;
	set_bound_t half_n = n / 2;
	limit = limit << half_n;

	for (sum_free_set_t i = 1; i < limit; i++) {
		sum_free_set_t set = i << 1;
		mirror_first_half_of_set(set, n);

		if(is_complete_sum_free(set, n)) {
			// print_set(set, n);
			//print_set(~set, n);
			std::cout << "found set\n";
			num_found++;
		}
		//        else if (is_complete_sum_free(new_set, n)) {
		//            printf("Found new complete sum free: ");
		//            print_set(new_set, n);
		//        }
	}
	printf("Number of sets: %d\n", num_found);

	return num_found;
}

void print_conversion(const uint32_t* set, set_bound_t n) {
	set_bound_t previous_index = 0;
	for (set_bound_t i = 0; i <= n; i++) {
		if (set[previous_index] == i) {
			printf("1");
			previous_index++;
		}
		else {
			printf("0");
		}
	}
}

sum_free_set_t convert(const uint32_t set[]) {
	sum_free_set_t out = 0;
	for (set_bound_t i = 0; set[i]; i++) {
		out |= ((sum_free_set_t) 1) << set[i];
		// std::cout << set[i] << ": " << std::bitset<42>(out) << '\n';
	}
	return out;
}



int main() {
	// Clear the graph file
#ifdef SUM_FREE_SETS_LOG_GRAPH
	std::ofstream("output_files/complete_sum_free_sets_log_graph_points", std::ios_base::trunc).close();
#endif


    print_all_complete_sum_free_sets_new(65);

	return 0;
}


    // 54: 25.60
    // 56: 51.47
    // 58: 101.18
    // 60: 208.49
    // 62: 406.89
    // Guess: 64: 840
    //
    // 61: 43.58
    // 63: 73.80

// 55:
//  Initial:
//      Found 656 ./complete_sum_free_sets_rewrite  19.30s user 0.00s system 99% cpu 19.321 tot
//  Optimizations 1:
//      Found 656 ./complete_sum_free_sets_rewrite  18.88s user 0.00s system 99% cpu 18.914 total
//  Optimizations 2:
//      Found: 656 ./complete_sum_free_sets_rewrite  7.41s user 0.00s system 99% cpu 7.445 total
//  Optimizations 3:
//      Found: 656 ./complete_sum_free_sets_rewrite  2.16s user 0.00s system 99% cpu 2.174 total
