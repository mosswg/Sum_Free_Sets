//
// Created by moss on 10/18/22.
//
#include "complete_sum_free_sets_consts.h"
#include <fstream>
#include <cstdint>
#include <bitset>
#include <cmath>

// https://en.wikipedia.org/wiki/Circular_shift
uint64_t rotl(uint64_t value, unsigned int count, int n) {
    return value << count | value >> (n - count);
}

void write_set_to_file(uint64_t set, int n, std::ofstream& outfile) {
    outfile << "{";
    for (int i = 0; i < n; i++) {
        if ((set >> i) & 0b1) {
            outfile << i;

            if ((set >> (i + 1))) {
                outfile << ", ";
            }
        }
    }
    outfile << "},\n";
}

uint32_t get_set_bits(uint64_t value) {
    uint32_t count = 0;
    while(value){
        value &= value-1;
        count++;
    }
    return count;
}

void print_set(uint64_t set, uint32_t n) {
    printf("{");
    for (int i = 0; i < n; i++) {
        if ((set >> i) & 0b1) {
            printf("%d", i);

            if ((set >> (i + 1))) {
                printf(", ");
            }
        }
    }
    printf("},\n");
}

uint64_t complete_sum_free_sets[15000];
uint32_t current_complete_sum_free_set = 0;
uint32_t minimum_found_set_length;
uint64_t mask;
uint64_t max_first_value;

void initialize_for_n(int n) {
    current_complete_sum_free_set = 0;
    minimum_found_set_length = n;
    max_first_value = (n + 1) / 3;
    mask = ((((uint64_t)1) << (n)) - 1);
}

void print_sets(uint32_t n) {
    uint64_t set;

    for (int i = 0; (set = complete_sum_free_sets[i]); i++) {
        print_set(set, n);
    }
    std::cout << '\n';
}

void write_sets_to_file(int n) {
    uint64_t set;
    std::ofstream outfile("output_files/complete_sum_free_sets_" + std::to_string(n));

    for (int i = 0; (set = complete_sum_free_sets[i]); i++) {
        write_set_to_file(set, n, outfile);
    }
    outfile.close();

    std::cout << '\n';
}

uint64_t generate_sums(uint64_t set, int n) {
    uint64_t sums = 0;

    for (int i = 1; set >> i; i++) {
        if ((set >> i) & 0b1) {
            sums |= rotl(set, i, n);
        }
    }

    return sums;
}

bool is_complete(const uint64_t set, int n) {
    uint64_t sums = 0;

    for (uint64_t i = 0; i <= n; i++) {
        // Add i to each value of the set and or with the sums if the set contains i
        if ((set >> i) & 0b1) {
            sums |= rotl(set, i, n);
        }
    }

    // Check if the sums are equal to the complementary set
    return ((sums & mask) == ((~set) & mask));
}

int get_last_node_value(uint64_t set) {
    int i = 1;
    for (; set >> i; i++);
    return i - 1;
}

int generate_sub_nodes(uint64_t set, uint64_t sums, uint32_t last_node_value, int n, uint64_t* sets_output, uint64_t* sums_output, uint32_t* new_node_output) {
    uint64_t copy;
    uint64_t sums_copy;
    int index = 0;


    for (int i = last_node_value + 1; i < n; i++) {
        if (i == last_node_value * 2) continue;
        copy = set | (((uint64_t)1) << i);


        sums_copy = sums | rotl(copy, i, n);

        if (!((sums_copy & (copy)) & mask)) {
            sets_output[index] = copy;
            sums_output[index] = (sums_copy & mask);
            new_node_output[index] = i;
            index++;
        }
    }

    return index;
}

void generate_nodes_from_set(uint64_t set, uint64_t sums, uint32_t last_node_value, int size, int n) {
    uint64_t new_sets[n];
    uint64_t new_sums[n];
    uint32_t new_last_node_values[n];
    int num_new_sets = generate_sub_nodes(set, sums, last_node_value, n, new_sets, new_sums, new_last_node_values);

    if (num_new_sets == 0) {
        if (((sums & mask) == ((~set) & mask))) {
            if (size < minimum_found_set_length) {
                minimum_found_set_length = size;
            }
            complete_sum_free_sets[current_complete_sum_free_set++] = set;
        }
        return;
    }

    for (int i = 0; i < num_new_sets; i++) {
        generate_nodes_from_set(new_sets[i], new_sums[i], new_last_node_values[i], size + 1, n);
    }
}

void generate_nodes_from_starting_value(int value, int n) {
    uint64_t starting_value = ((uint64_t) 0b1) << value;
    generate_nodes_from_set(starting_value, rotl(starting_value, value, n), value, 1, n);
}

void generate_complete_sum_free_sets(int n) {
    for (int i = 1; i <= max_first_value; i++) {
        std::cout << i << ' ' << std::flush;
        generate_nodes_from_starting_value(i, n);
    }
    std::cout << std::endl;
}


bool is_complete_sum_free(const uint64_t set, int n) {
    uint64_t sums = 0;

    for (uint64_t i = 0; i <= n; i++) {
        // Add i to each value of the set and or with the sums if the set contains i
        if ((set >> i) & 0b1) {
            sums |= rotl(set, i, n);

            // Check if not sum-free
            if ((sums & (set)) & mask) {
                return false;
            }
        }
    }

    print_set(set, n);
    print_set(sums & mask, n);



    // Check if the sums are equal to the complementary set
    return ((sums & mask) == ((~set) & mask));
}

int print_all_complete_sum_free_sets(int n) {
    int num_found = 0;
    uint64_t limit = 1;
    limit = limit << (n - 1);

    for (uint64_t i = 1; i < limit; i++) {
        uint64_t set = i << 1;

        if(is_complete_sum_free(set, n)) {
            num_found++;
//            print_set(set, n);
        }
    }


    return num_found;
}



double log_10_2 = log10(2);
uint32_t print_all_complete_sum_free_sets_new(int n) {
    initialize_for_n(n);
    std::cout << "Generating Sum Free Sets" << std::endl;
    generate_complete_sum_free_sets(n);

    print_sets(n);

    std::ofstream("output_files/complete_sum_free_sets_graph", std::ios_base::app) << "(" << n << "," << (log10(current_complete_sum_free_set) / log_10_2) << ")\n";

    std::cout << "Found: " << current_complete_sum_free_set << std::endl;
    std::cout << "Minimum Set Length: " << (minimum_found_set_length == n ? 0 : minimum_found_set_length) << std::endl;

    return current_complete_sum_free_set;
}

int print_symmetric_complete_sum_free_sets(int n) {
    initialize_for_n(n);
    int num_found = 0;
    uint64_t limit = 1;
    int half_n = n / 2;
    limit = limit << half_n;

    for (uint64_t i = 1; i < limit; i++) {
        uint64_t set = i << 1;
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

void print_conversion(const int32_t* set, int n) {
    int previous_index = 0;
    for (int i = 0; i <= n; i++) {
        if (set[previous_index] == i) {
            printf("1");
            previous_index++;
        }
        else {
            printf("0");
        }
    }
}

uint64_t convert(const int32_t set[]) {
    uint64_t out = 0;
    for (int i = 0; set[i]; i++) {
        out |= ((uint64_t) 1) << set[i];
         // std::cout << set[i] << ": " << std::bitset<42>(out) << '\n';
    }
    return out;
}


void test_against_known_values(int n) {
    uint32_t num_found = print_all_complete_sum_free_sets_new(n);

    uint32_t size = number_of_sets[n];

    if (num_found != size) {
        std::cerr << "ERROR: Size mismatch in results:\n\tFound: " << num_found << "\n\tShould Find: " << size << std::endl;
        exit(1);
    }

    bool* is_found = new bool[size];
    bool missing_any = false;
    uint64_t set;

    for (int i = 0; i < size; i++) {
        uint64_t converted_set = convert(get_known_complete_sum_free_set(n, i));
        for (int j = 0; (set = complete_sum_free_sets[j]); j++) {
            if (set == converted_set) {
                is_found[i] = true;
                break;
            }
        }

        if (!is_found[i]) {
            missing_any = true;
            std::cout << "Not Found: ";
            print_set(converted_set, n);
        }
    }

    if (!missing_any) {
        std::cout << "All sets for n = " << n << " found\n";
    }
    else {
        exit(1);
    }

    delete[] is_found;
}


int main() {

	for (int n = 2; n <= 60; n++) {
		print_all_complete_sum_free_sets_new(n);
	}
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
