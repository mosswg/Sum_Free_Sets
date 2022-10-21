//
// Created by moss on 10/18/22.
//
#include "complete_sum_free_sets_consts.h"
#include <iostream>
#include <fstream>

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

void print_set(uint64_t set, int n) {
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

uint64_t complete_sum_free_sets[10000];
uint32_t current_complete_sum_free_set = 0;
uint32_t minimum_found_set_length;
uint64_t mask;

void print_sets(int n) {
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

int generate_sub_nodes(uint64_t set, uint64_t sums, int n, uint64_t* sets_output, uint64_t* sums_output) {
    uint64_t copy;
    uint64_t sums_copy;
    int last_node_value = get_last_node_value(set);
    int index = 0;


    for (int i = last_node_value + 1; i < n; i++) {
        if (i == last_node_value * 2) continue;
        copy = set | (((uint64_t)1) << i);


        sums_copy = sums | rotl(copy, i, n);

        if (!((sums_copy & (copy)) & mask)) {
            sets_output[index] = copy;
            sums_output[index++] = (sums_copy & mask);
        }
    }

    return index;
}

void generate_nodes_from_set(uint64_t set, uint64_t sums, int size, int n) {
    uint64_t new_sets[n];
    uint64_t new_sums[n];
    int num_new_sets = generate_sub_nodes(set, sums, n, new_sets, new_sums);

    if (num_new_sets == 0) {
        uint32_t sums_size = get_set_bits(sums);
        if ((n - size) == sums_size) {
            if (is_complete(set, n)) {
                if (size < minimum_found_set_length) {
                    minimum_found_set_length = size;
                }
                complete_sum_free_sets[current_complete_sum_free_set++] = set;
            }
            else {
//                std::cout << size << ' ' << sums_size << std::endl;
//                print_set(set, n);
            }
        }
        return;
    }

    for (int i = 0; i < num_new_sets; i++) {
        generate_nodes_from_set(new_sets[i], new_sums[i], size + 1, n);
    }
}

void generate_complete_sum_free_sets(int n) {
    for (int i = 1; i <= ((n + 1) / 3); i++) {
        std::cout << i << ' ' << std::flush;
        generate_nodes_from_set(((uint64_t) 0b1) << i, ((uint64_t) 0b1) << (i*2), 1, n);
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


uint32_t print_all_complete_sum_free_sets_new(int n) {
    minimum_found_set_length = n;
    mask = (((((uint64_t)1) << (n - 1)) - 1) << 1) | 0b1;
    std::cout << "Generating Sum Free Sets" << std::endl;
    generate_complete_sum_free_sets(n);

    write_sets_to_file(n);

    std::cout << "Found: " << current_complete_sum_free_set << std::endl;
    std::cout << "Minimum Set Length: " << (minimum_found_set_length == n ? 0 : minimum_found_set_length) << std::endl;

    return current_complete_sum_free_set;
}



void create_set(uint64_t& set, int n) {
    uint64_t out = 0;
    for (int i = 0; i <= (n / 2); i++) {
        set |= ((set >> i) & 0b1) << (n - i);
    }
}

int print_symmetric_complete_sum_free_sets(int n) {
    int num_found = 0;
    uint64_t limit = 1;
    int half_n = n / 2;
    limit = limit << half_n;

    for (uint64_t i = 1; i < limit; i++) {
        uint64_t set = i << 1;
        create_set(set, n);
        uint64_t new_set = i << 1;
        new_set |= new_set << half_n;

        if(is_complete_sum_free(set, n)) {
            // print_set(set, n);
            print_set(~set, n);
            std::cout << std::endl;
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

int main() {


    // 54: 25.60
    // 56: 51.47
    // 58: 101.18
    // 60: 208.49
    // 62: 406.89
    // Guess: 64: 840
    //
    // 61: 43.58
    // 63: 73.80


    for (int n = 61; n <= 63; n++) {
        std::cout << "\n\nn = " << n << std::endl;
        print_all_complete_sum_free_sets_new(n);

        for (int i = 0; complete_sum_free_sets[i]; i++) {
            complete_sum_free_sets[i] = 0;
        }
        current_complete_sum_free_set = 0;
    }

//    int found = sizeof(sets_37) / sizeof(sets_37[0]);
//    bool* is_found = new bool[found];
//    uint64_t set;
//
//    for (int i = 0; i < found; i++) {
//        uint64_t converted_set = convert(sets_37[i]);
//        for (int j = 0; (set = complete_sum_free_sets[j]); j++) {
//            if (set == converted_set) {
//                is_found[i] = true;
//                break;
//            }
//        }
//
//        if (!is_found[i]) {
//            std::cout << "Not Found: " << std::bitset<37>(converted_set) << '\t';
//            print_set(converted_set, n);
////            exit(1);
//        }
//    }
//
//    delete[] is_found;

    return 0;

}


/*
 {1},
{2},
{3},
{1, 3},
{2, 3},
{4},
{3, 4},
{5},
{1, 5},
{2, 5},
{4, 5},
{6},
{1, 6},
{2, 6},
{4, 6},
 */







//  Found 656 ./complete_sum_free_sets_rewrite  19.30s user 0.00s system 99% cpu 19.321 tot
//  Found 656 ./complete_sum_free_sets_rewrite  18.88s user 0.00s system 99% cpu 18.914 total





//    for (int n = 2; n <= 10; n++) {
//        printf("\n\nn = %d\n", n);
//        print_complete_sum_free_sets(n);
//    }

//    for (int i = 0; i < found; i++) {
//        uint64_t converted_set = convert(sets_36[i]);
//        for (unsigned long j : sets_fast) {
//            if (j == converted_set) {
//                is_found[i] = true;
//                break;
//            }
//            else if (j) {
////                std::cout << "Conval NEQ fval: \n\t" << std::bitset<37>(j) << '\t';
////                print_set(j, n);
////                std::cout << "\t" << std::bitset<37>(converted_set) << '\t';
////                print_set(converted_set, n);
//            }
//        }
//
//        if (!is_found[i]) {
//            std::cout << "Not Found: " << std::bitset<37>(converted_set) << '\t';
//            print_set(converted_set, n);
////            exit(1);
//        }
//    }


//    const int n = 51;

//    int set[] = {1, 4, 10, 17, 19, 24, 26, 32, 35};
//
//    std::cout << std::bitset<n>(convert(set)) << std::endl;
//
//    return 0;

//    int found = sizeof(sets_51) / sizeof(sets_51[0]);
//
//
//    for (int i = 0; i < found; i++) {
//        for (int& j : doubles) {
//            j = 0;
//        }
//
//        for (int32_t value : sets_51[i]) {
//            if (!value) {
//                break;
//            }
//
//            for (int32_t double_val : doubles) {
//                if (value == double_val) {
//                    printf("Rule break: %d in ", value);
//                    print_set(convert(sets_51[i]), n);
//                }
//            }
//        }
//    }