//
// Created by moss on 10/18/22.
//

#include <cstdint>
#include <iostream>

// https://en.wikipedia.org/wiki/Circular_shift
uint64_t rotl(uint64_t value, unsigned int count, int n) {
    return value << count | value >> (n - count);
}

void print_set(uint64_t set, int n) {
    for (int i = 0; i < n; i++) {
        if ((set >> i) & 0b1) {
            std::cout << i << ' ';
        }
    }
    std::cout << '\n';
}

bool is_complete_sum_free(const uint64_t set, int n) {
    uint64_t sums = 0;
    uint64_t mask = ((((uint64_t)1) << n) - 1);

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

void print_complete_sum_free_sets(int n) {
    uint64_t num_checked = 0;
    uint64_t limit = 1;
    limit = limit << (n - 1);

    for (uint64_t i = 0; i < limit; i++) {

        uint64_t set = i << 1;

        if(is_complete_sum_free(set, n)) {
            print_set(set, n);
        }
    }

    std::cout << num_checked << '\n';
}

int main() {
    int n = 10;

    print_complete_sum_free_sets(n);
}

/*
9 11 13 14 15 16 17 19 21
6 9 13 14 16 17 21 24
6 7 9 11 19 21 23 24
5 6 13 14 15 16 17 24 25
5 6 7 8 9 21 22 23 24 25
4 6 7 9 21 23 24 26
4 5 6 7 15 23 24 25 26
3 11 12 13 17 18 19 27
3 7 8 9 21 22 23 27
3 4 9 14 15 16 21 26 27
3 4 9 11 19 21 26 27
3 4 5 14 15 16 25 26 27
3 4 5 11 12 18 19 25 26 27
2 5 11 12 15 18 19 25 28
2 5 8 9 15 21 22 25 28
2 5 6 9 13 17 21 24 25 28
2 3 11 12 18 19 27 28
2 3 9 13 17 21 27 28
2 3 8 9 15 21 22 27 28
2 3 7 11 15 19 23 27 28
2 3 7 8 12 13 17 18 22 23 27 28
1 6 9 13 17 21 24 29
1 5 8 12 15 18 22 25 29
1 4 7 9 15 21 23 26 29
1 4 6 9 11 14 16 19 21 24 26 29
1 3 9 14 16 21 27 29
1 3 8 13 15 17 22 27 29
1 3 8 12 18 22 27 29
1 3 7 12 18 23 27 29
1 3 5 12 14 16 18 25 27 29
1 3 5 7 9 11 13 15 17 19 21 23 25 27 29
 */