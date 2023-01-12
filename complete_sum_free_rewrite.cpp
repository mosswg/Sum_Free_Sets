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
	std::ofstream outfile;
	if (n < 10) {
		outfile.open("output_files/complete_sum_free_sets_0" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}
	else {
		outfile.open("output_files/complete_sum_free_sets_" + std::to_string(n), std::ofstream::out | std::ofstream::trunc);
	}


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

void mirror_first_half_of_set(uint64_t& set, int n) {
    for (int i = 0; i <= (n / 2); i++) {
        set |= ((set >> i) & 0b1) << (n - i);
    }
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

    // Check if the sums are equal to the complementary set
    return ((sums & mask) == ((~set) & mask));
}

__always_inline uint64_t& get_gp_value(uint64_t* gps, uint32_t i, uint32_t j, uint32_t n) {
    return gps[((n * i) + j)];
}

__always_inline void set_gp_value(uint64_t* gps, uint64_t value, uint32_t i, uint32_t j, uint32_t n) {
    gps[((n * i) + j)] = value;
}

void generate_gp(uint32_t n, uint32_t i, uint32_t& current_me_index, uint64_t* me, uint64_t* output) {
    constexpr uint64_t one = ((uint64_t)1);
    for (uint32_t j = i + 1; j < n; j++) {
        output[j] = 0;
        uint32_t j_inv = ((n + i) - j);
        if (j_inv == j) {
            me[current_me_index++] = (one << j) | (one << i);
        }
        output[j] |= (one << j_inv);
        output[j] |= (one << j);
    }
    uint64_t j_mask;
    uint64_t n_j_mask;
    uint64_t current_mask;
    bool invalid_current_value;
    for (uint32_t j = 1; j < i; j++) {
        invalid_current_value = false;
        j_mask = (one << (j));
        n_j_mask = (one << (i - j));
        if (j_mask == n_j_mask) {
            me[current_me_index++] = (one << j) | (one << i);
        }
        current_mask = j_mask | n_j_mask;
        for (int k = 0; k < current_me_index; k++) {
            if (me[k] == current_mask) {
                invalid_current_value = true;
                break;
            }
        }
        if (!invalid_current_value) {
            output[j] = 0;
            output[j] = current_mask;
        }
    }
}

void remove_invalid_gps(uint64_t* gps, const uint64_t* me, uint32_t num_me, uint32_t n) {
    for (uint32_t i = 1; i < n; i++) {
        for (uint32_t j = 1; j < n; j++) {
            for (int k = 0; k < num_me; k++) {
                if (me[k] == get_gp_value(gps, i, j, n)) {
                    set_gp_value(gps, 0, i, j, n);
                    break;
                }
            }
        }
    }
}

void remove_duplicate_meps(uint64_t* me, uint32_t& current_me_index) {
    bool is_last_value = true;
    bool removed = false;
    for (int32_t i = (int32_t)current_me_index - 1; i > 0; i--) {
        for (int32_t j = 0; j < i; j++) {
            if (me[i] == me[j]) {
                if (is_last_value) {
                    me[i] = 0;
                    current_me_index--;
                    removed = true;
                    break;
                }
                else {
                    for (int32_t k = i + 1; k < current_me_index; k++) {
                        me[k - 1] = me[k];
                    }
                    me[--current_me_index] = 0;
                    is_last_value = true;
                }
            }
        }
        if (!removed) {
            is_last_value = false;
        }
        removed = false;
    }
}

bool is_eliminated(uint64_t elimination, uint32_t value) {
    return elimination & (((uint64_t)1) << value);
}

bool eliminate_value_from_gps(uint64_t& set, uint64_t& elimination, uint64_t* gps, uint64_t* me, uint32_t& current_me_index, uint32_t value_to_remove, uint32_t n) {
    elimination |= ((uint64_t)1) << value_to_remove;
    uint64_t value_mask = ((uint64_t)1) << value_to_remove;

    for (int i = 1; i < n; i++) {
        if (is_eliminated(elimination, i)) {
            // This indicates that the value has already been eliminated
            continue;
        }
        uint32_t num_values_in_gps = 0;
        uint64_t previous_val = 0;
        for (int j = 1; j < n; j++) {
            uint64_t& current_gp_value = get_gp_value(gps, i, j, n);
            if (current_gp_value && current_gp_value != previous_val) {
                num_values_in_gps++;
                previous_val = current_gp_value;
            }
        }
        if (num_values_in_gps == 1) {
            for (int j = 1; j < n; j++) {
                if ((get_gp_value(gps, i, j, n) & value_mask)) {
                    set |= (((uint64_t)1) << i);
                    if (set & elimination) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool remove_value_from_gps(uint64_t& set, uint64_t& elimination, uint64_t* gps, uint64_t* me, uint32_t& current_me_index, uint32_t value_to_remove, uint32_t n, uint32_t max_value = 0) {
    uint64_t value_mask = ((uint64_t)1) << value_to_remove;
    if (!max_value) {
        max_value = n;
    }
    uint64_t mep;
    for (int i = 1; i < n; i++) {
        if (is_eliminated(elimination, i)) {
            continue;
        }
        if ((mep = get_gp_value(gps, value_to_remove, i, n))) {
            me[current_me_index++] = mep;
            // If mep has only one value. alt: if mep is a power of two
            if (!((mep) & (mep - 1))) {
                elimination |= mep;
            }
            if ((mep & set) == mep) {
                return false;
            }
        }
    }

    for (int i = 1; i < max_value; i++) {
        for (int j = 1; j < n; j++) {
            uint64_t& current_gp_value = get_gp_value(gps, i, j, n);
            if ((current_gp_value & value_mask)) {
                // Remove the value but leave everything else
                // Since gps are always either two numbers or one number we check that then operate accordingly
                current_gp_value &= ~value_mask;

                if (current_gp_value != 0) {
                    int left_over_value = 0;
                    for (; current_gp_value >> (left_over_value + 1); left_over_value++);
                    uint64_t new_me = (((uint64_t) 1) << i) | (((uint64_t) 1) << left_over_value);
                    bool add_me = true;
                    for (int k = 0; k < current_me_index; k++) {
                        if (me[k] == new_me) {
                            add_me = false;
                            break;
                        }
                    }
                    if (add_me) {
                        me[current_me_index++] = new_me;
                    }

                }
                else {
                    eliminate_value_from_gps(set, elimination, gps, me, current_me_index, i, n);
                    if (set & elimination) {
                        return false;
                    }
                }
            }
        }
    }
    remove_duplicate_meps(me, current_me_index);
    remove_invalid_gps(gps, me, current_me_index, n);
    return true;
}

bool check_if_could_be_valid(int check_value, uint64_t set, uint64_t &elimination, uint64_t **gps, uint64_t *me,
                             uint32_t &current_me_index, uint32_t n) {

}

bool logical_elimination(uint64_t& set, uint64_t& elimination, uint64_t *gps, uint64_t* me, uint32_t& current_me_index, uint32_t n, uint32_t max_value = 0) {
    bool empty_gps;
    if (!max_value) {
        max_value = n;
    }

    for (int i = 1; i < max_value; i++) {
        if (is_eliminated(elimination, i)) {
            continue;
        }
        empty_gps = true;
        for (int j = 1; j < n; j++) {
            if (gps[(n * i) + j] != 0) {
                // remove invalid gps
                uint64_t current_gp_value = get_gp_value(gps, i, j, n);
                if ((current_gp_value & elimination) == current_gp_value) {
                    current_gp_value = 0;
                    continue;
                }
                empty_gps = false;
                break;
            }
        }

        if (empty_gps) {
            set |= (((uint64_t)1) << i);
            bool valid_set = remove_value_from_gps(set, elimination, gps, me, current_me_index, i, n, max_value);
            if (!valid_set) {
                return false;
            }
        }
    }
    return true;
}


bool solve_set(uint64_t& set, uint64_t& elimination, uint64_t* gps, uint64_t* me, uint32_t& current_me_index, uint32_t n, uint32_t previous_recursion_start = 0) {
    if (!mask) {
        initialize_for_n(n);
    }
    uint64_t previous_set = set;
    // Valid set condition
    while (((set | elimination) | 0b1) != mask) {
//        std::cout << "Logical Elimination from: \n\t";
//        print_set(set, n);
        bool valid_set = logical_elimination(set, elimination, gps, me, current_me_index, n, previous_recursion_start);
//        std::cout << "\t";
//        print_set(set, n);
//        std::cout << "\tE: ";
//        print_set(elimination, n);
        // Invalid set condition
        if (!valid_set) {
            return false;
        }
        else if(is_complete_sum_free(set, n)) {
            std::cout << "found set " << current_complete_sum_free_set + 1 << " with elimination: ";
            print_set(set, n);
            complete_sum_free_sets[current_complete_sum_free_set++] = set;
            return true;
        }
        // Branching condition
        else if (set == previous_set) {
            uint64_t gps_copy[n * n];
            uint64_t me_copy[n * n];
            uint64_t set_copy;
            uint64_t elimination_copy;
            uint32_t current_me_copy;
            bool added_set = false;
            if (previous_recursion_start == 0) {
                previous_recursion_start = n;
            }
            for (uint32_t i = previous_recursion_start - 1; (previous_recursion_start != n && i > 0) || (previous_recursion_start == n && i > (n / 2)); i--) {
                uint64_t i_mask = (((uint64_t)1) << i);
                // don't try to add eliminated values
                if (i_mask & elimination) {
                    continue;
                }
                // avoid infinite recursion
                else if (i_mask & set) {
                    continue;
                }
                set_copy = set;
                elimination_copy = elimination;
                current_me_copy = current_me_index;
                for (int j = 0; j < n; j++) {
                    for (int k = 0; k < n; k++) {
                        set_gp_value(gps_copy, get_gp_value(gps, j, k, n), j, k, n);
                    }
                    me_copy[j] = me[j];
                }
                set_copy |= i_mask;
                // Avoid infinite recursion
                if (set_copy == set) continue;
                bool new_is_valid_set = remove_value_from_gps(set_copy, elimination_copy, gps_copy, (uint64_t*)me_copy, current_me_copy, i, n, i);
                if (new_is_valid_set) {
//                    std::cout << "If " << i << " in ";
//                    print_set(set, n);
//                    std::cout << "With elimination: ";
//                    print_set(elimination, n);
                    new_is_valid_set = solve_set(set_copy, elimination_copy, gps_copy, (uint64_t*)me_copy, current_me_copy, n, i);
                }
                if (new_is_valid_set && is_complete_sum_free(set_copy, n)) {
                    bool add_set = true;
                    for (int j = 0; j < current_complete_sum_free_set; j++) {
                        if (complete_sum_free_sets[j] == set_copy) {
                            add_set = false;
                            break;
                        }
                    }
                    if (add_set) {
                        added_set = true;
                        std::cout << "found set " << current_complete_sum_free_set + 1 << " with branch: ";
                        print_set(set_copy, n);
                        complete_sum_free_sets[current_complete_sum_free_set++] = set_copy;
                    }
                }
            }
//            std::cout << "decursing with: ";
//            print_set(set, n);
            return added_set;
        }
        previous_set = set;
    }

    return true;
}

void print_gps(uint64_t** gps, uint32_t n) {
    for (uint32_t i = 0; i < n; i++) {
        std::cout << "gp_" << i << " = {\n";
        for (int j = 0; j < n; j++) {
            print_set(gps[i][j], n);
        }
        std::cout << "}\n";
    }
}

void print_meps(uint64_t* me, uint32_t num_me, uint32_t n) {

    std::cout << "\nme = {\n";
    for (int j = 0; j < num_me; j++) {
        print_set(me[j], n);
    }
    std::cout << "}\n";
}

uint32_t generate_gps(uint32_t n, uint64_t* me, uint64_t* output) {
    for (int i = 0; i < n; i++) {
        me[i] = 0;
    }
    uint32_t current_me_index = 0;
    for (uint32_t i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            set_gp_value(output, 0, i, j, n);
        }
        generate_gp(n, i, current_me_index, me, output + (i * n));

    }

    remove_invalid_gps(output, me, current_me_index, n);

    return current_me_index;
}

bool check_for_asymmetric_sets(uint32_t n) {
    uint64_t mask = (((uint64_t)1) << ((n + 1) / 2)) - 1;
    bool found = false;
    for (uint32_t i = 0; i < current_complete_sum_free_set; i++) {
		uint64_t current_set = complete_sum_free_sets[i];
		if (!current_set) {
			continue;
		}
        uint64_t half_set = current_set & mask;
        mirror_first_half_of_set(half_set, n);
        if (half_set != current_set) {
            std::cout << "Found asymmetric set" << std::endl;
            print_set(current_set, n);
            found = true;
        }
    }
    return found;
}

uint32_t print_all_complete_sum_free_sets_logic(uint32_t n) {
    uint64_t gps[n*n];

    uint64_t me[n*n];

    uint64_t set = 0, elimination = 0;

    uint32_t num_me = generate_gps(n, me, gps);

    solve_set(set, elimination, gps, me, num_me, n);

    std::cout << "\nFound: \n";
    print_sets(n);

    std::cout << "Found asymmetric?: \n";

    if(!check_for_asymmetric_sets(n)) {
        std::cout << "No";
    }

    std::cout << "Found: " << current_complete_sum_free_set << std::endl;

    return current_complete_sum_free_set;
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

    std::ofstream("output_files/complete_sum_free_sets_graph", std::ios_base::out | std::ios_base::trunc) << "(" << n << "," << (log10(current_complete_sum_free_set) / log_10_2) << ")\n";

	write_sets_to_file(n);

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
    uint32_t num_found = print_all_complete_sum_free_sets_logic(n);

    uint32_t size = number_of_sets[n];

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

	for (int n = 2; n <= 63; n++) {
		print_all_complete_sum_free_sets_new(n);
		for (int i = 0; i < current_complete_sum_free_set; i++) {
			complete_sum_free_sets[i] = 0;
		}
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
