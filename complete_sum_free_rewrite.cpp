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
// #define TIME_SUM_FREE_SETS_GENERATION
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

const set_bound_t num_complete_per_thread = 20000;
set_bound_t descision_tree_thread_bound_length;
set_bound_t descision_tree_thread_bound;
set_bound_t num_threads;
sum_free_set_t** complete_sum_free_sets_for_thread;
set_bound_t* current_complete_sum_free_set_for_thread;
set_bound_t total_number_of_complete_sum_free_sets;
set_bound_t minimum_found_set_length;
sum_free_set_t mask;
set_bound_t max_first_value;
std::vector<std::thread> global_threads;

const std::string base_output_folder = "output";

void initialize_for_n(set_bound_t n) {
	minimum_found_set_length = n;
	max_first_value = ((n + 1) / 3);
	mask = ((((sum_free_set_t)1) << (n)) - 1);

	descision_tree_thread_bound_length = 10;
	if (n < 45) {
		descision_tree_thread_bound_length = (n / 5) + 1;
	}
	descision_tree_thread_bound = (1 << descision_tree_thread_bound_length);
	num_threads = descision_tree_thread_bound << 1;

	complete_sum_free_sets_for_thread = new sum_free_set_t*[num_threads];
	current_complete_sum_free_set_for_thread = new set_bound_t[num_threads];

	for(int i = 0; i < num_threads; i++) {
		current_complete_sum_free_set_for_thread[i] = 0;
		complete_sum_free_sets_for_thread[i] = new sum_free_set_t[num_complete_per_thread];
		for (int j = 0; j < num_complete_per_thread; j++) {
			complete_sum_free_sets_for_thread[i][j] = 0;
		}
	}

}

void print_sets(set_bound_t n) {
	sum_free_set_t set;

	for (set_bound_t thread_index = 0; thread_index < descision_tree_thread_bound; thread_index++) {
		for (set_bound_t i = 0; (set = complete_sum_free_sets_for_thread[thread_index][i]); i++) {
			print_set(set, n);
		}
	}
	std::cout << '\n';
}

void write_sets_to_file(set_bound_t n, double compute_time = 0) {
	sum_free_set_t set;
	std::ofstream outfile;
	if (n < 10) {
		outfile.open(base_output_folder + "/complete_sum_free_sets_0" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}
	else {
		outfile.open(base_output_folder + "/complete_sum_free_sets_" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}


	#ifdef SUM_FREE_SETS_GRAPH_OUTPUT
	std::ofstream graph_outfile;
	if (n < 10) {
		graph_outfile.open("tree_files/complete_sum_free/0" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}
	else {
		graph_outfile.open("tree_files/complete_sum_free/" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}

	graph_outfile << "digraph BST {\n";
	#endif


	for (set_bound_t thread_index = 0; thread_index < descision_tree_thread_bound; thread_index++) {
		for (set_bound_t i = 0; (set = complete_sum_free_sets_for_thread[thread_index][i]); i++) {
			write_set_to_file(set, n, outfile);
			#ifdef SUM_FREE_SETS_GRAPH_OUTPUT
			write_set_tree_to_file(set, n, graph_outfile);
			#endif
		}
	}
	if (compute_time != 0) {
		outfile << "Time: " << compute_time << " ms\n";
	}
	outfile.close();

	#ifdef SUM_FREE_SETS_GRAPH_OUTPUT
	graph_outfile << "}";
	graph_outfile.close();
	#endif

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

bool is_complete_sum_free(const sum_free_set_t set, const sum_free_set_t sums, set_bound_t n) {
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

bool pre_thread_check_for_children(sum_free_set_t set, sum_free_set_t sums, set_bound_t last_node_value, set_bound_t n) {
	sum_free_set_t new_set;
	sum_free_set_t new_set_sums;
	sum_free_set_t new_value_to_add;
	set_bound_t double_last_node_value = last_node_value * 2;
	for (set_bound_t i = last_node_value + 1; i < n; i++) {
		if (i == double_last_node_value) continue;
		new_value_to_add = ((sum_free_set_t)1) << i;

		// Ignore any values that add
		if (new_value_to_add & sums & mask) continue;

		new_set = set | new_value_to_add;

		new_set_sums = sums | rotl(new_set, i, n);

		if (!((new_set_sums & (new_set)) & mask)) {
			return true;
		}
	}
	return false;
}


void generate_set_from_descision_tree(sum_free_set_t descision_tree, set_bound_t descision_tree_length, set_bound_t n, sum_free_set_t& set, sum_free_set_t& sums, set_bound_t& last_node_value) {
	set = 0;
	sums = 0;
	sum_free_set_t new_set = 0;
	sum_free_set_t new_sums = 0;
	sum_free_set_t new_value_to_add;
	set_bound_t double_last_added_value = 0;
	set_bound_t descision_index = 0;
	for (set_bound_t i = 1; i < n && descision_index < descision_tree_length; i++) {
		if (i == double_last_added_value) continue;
		new_value_to_add = ((sum_free_set_t)1) << i;

		// Ignore any values that add
		if (new_value_to_add & sums) continue;

		new_set = set | new_value_to_add;

		new_sums = sums | rotl(new_set, i, n);

		if (!((new_sums & (new_set)))) {
			if (descision_tree & (1 << descision_index++)) {
				set = new_set;
				sums = new_sums;
			}
			last_node_value = i;
		}
	}
}

uint32_t generate_sub_nodes(sum_free_set_t set, sum_free_set_t sums, set_bound_t last_node_value, set_bound_t n, set_bound_t thread_index) {
	sum_free_set_t new_set;
	sum_free_set_t new_set_sums;
	sum_free_set_t new_value_to_add;
	set_bound_t number_of_new_sum_free_sets = 0;
	set_bound_t double_last_node_value = last_node_value * 2;


	for (set_bound_t i = last_node_value + 1; i < n; i++) {
		new_value_to_add = ((sum_free_set_t)1) << i;

		// Ignore any values that add
		if (new_value_to_add & sums) continue;

		new_set = set | new_value_to_add;

		new_set_sums = sums | rotl(new_set, i, n);

		if (!((new_set_sums & (new_set)))) {
			int num_generated = generate_sub_nodes(new_set, new_set_sums, i, n, thread_index);
			if (num_generated == 0) {
				if ((new_set_sums & mask) == ((~new_set) & mask)) {
					complete_sum_free_sets_for_thread[thread_index][current_complete_sum_free_set_for_thread[thread_index]++] = new_set;
				}
			}
			number_of_new_sum_free_sets++;
		}
	}

	return number_of_new_sum_free_sets;
}

void generate_nodes_from_set(sum_free_set_t set, sum_free_set_t sums, set_bound_t last_node_value, set_bound_t size, set_bound_t n, set_bound_t thread_index) {
	sum_free_set_t decision_tree = set;
	uint32_t num_new_sets = generate_sub_nodes(set, sums, last_node_value, n, thread_index);

	if (num_new_sets == 0) {
		if (((sums) == ((~set) & mask))) {
			if (size < minimum_found_set_length) {
				minimum_found_set_length = size;
			}
			complete_sum_free_sets_for_thread[thread_index][current_complete_sum_free_set_for_thread[thread_index]++] = set;
		}
	}

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


#ifdef SUM_FREE_SETS_GRAPH_OUTPUT
	outfile << "}";
	outfile.close();
#endif
}

/// threads.emplace_back(generate_nodes_from_starting_value, i, n);

void generate_complete_sum_free_sets(set_bound_t n) {
	sum_free_set_t set, sums;
	set_bound_t last_node_value;
	bool set_already_checked = false;
	std::cout << "creating threads...\n" << std::flush;
	for (set_bound_t i = 0; i < descision_tree_thread_bound; i++) {
		generate_set_from_descision_tree(i, descision_tree_thread_bound_length, n, set, sums, last_node_value);
		if(is_complete_sum_free(set, sums, n)) {
			complete_sum_free_sets_for_thread[i][current_complete_sum_free_set_for_thread[i]++] = set;
			// if the set is already complete sum free add to the set and don't branch (adding to a complete sum free set will always make it non-complete).
			continue;
		}
		// std::cout << "threading on: ";
		//print_set(set, n);
		global_threads.emplace_back(generate_sub_nodes, set, sums, last_node_value, n, i);

	}
	std::cout << "waiting for threads...\n" << std::flush;

	/// Ensure that threads are done before exiting
	int thread_index = 0;
	for (auto& thread : global_threads) {
		if (thread.joinable()) {
			thread.join();
		}
		thread_index++;
	}

	total_number_of_complete_sum_free_sets = 0;
	for (set_bound_t i = 0; i < num_threads; i++) {
		total_number_of_complete_sum_free_sets += current_complete_sum_free_set_for_thread[i];
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

	delete[] current_complete_sum_free_set_for_thread;

	for(int i = 0; i < num_threads; i++) {
		delete[] complete_sum_free_sets_for_thread[i];
	}
	delete[] complete_sum_free_sets_for_thread;

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

void print_usage(std::string filename) {
	std::cout << "usage: " << filename << " [rt] <n>\n";
	std::cout << "\trt: s for single value of n or r for range from 1 to n\n";
}


int main(int argc, char** argv) {
	// Clear the graph file
#ifdef SUM_FREE_SETS_LOG_GRAPH
	std::ofstream("output_files/complete_sum_free_sets_log_graph_points", std::ios_base::trunc).close();
#endif

	if (argc < 2) {
		print_usage(argv[0]);
		return 1;
	}

	if (argc == 2) {
		set_bound_t n = std::stoi(argv[1]);

		print_all_complete_sum_free_sets_new(n);
	}
	else if (std::string(argv[1]) == "s") {
		set_bound_t n = std::stoi(argv[2]);

		print_all_complete_sum_free_sets_new(n);
	}
	else if (std::string(argv[1]) == "r") {
		set_bound_t n = std::stoi(argv[2]);

		for (set_bound_t i = 1; i <= n; i++) {
			std::cout << "n=" << i << "\n";
			print_all_complete_sum_free_sets_new(i);
		}
	}
	else {
		print_usage(argv[0]);
	}

	return 0;
}
