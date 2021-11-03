//
// Copyright (c) 2021 Moss Gallagher.
//
#pragma once

#include <iostream>
#include <algorithm>
#include <vector>

template <typename t> std::ostream& operator<<(std::ostream& out, const std::vector<t>& print) {
	out << "{";

	if (print.empty()) {
		out << "}";
		return out;
	}

	out << print.front();

	for(int i = 1; i < print.size(); i++) {
		out << ", ";
		out << print[i];
	}

	out << "}";

	return out;
}

template <typename t> bool vector_contains(std::vector<t> const& vec, t const& search) {
	return std::any_of(vec.begin(), vec.end(), [search](auto const& element){return element == search;});
}

template <typename t> bool is_sum_free(std::vector<t> const& set, std::vector<t> const& sums) {
	return !std::any_of(sums.begin(), sums.end(), [set](auto const& sum){ return vector_contains(set, sum); });
}

template <typename t> bool is_complete(std::vector<t> const& set, std::vector<t> const& sums, t n) {
	std::vector<int> complementary(n);

	t j = 0;
	for(t i = 0; i < n; i++) {
		if (!vector_contains(set, i)) {
			complementary[j] = i;
			j++;
		}
	}

	complementary.resize(j);
	std::vector<bool> is_found(j);

	for(auto const& sum : sums) {
		auto index = std::find(complementary.begin(), complementary.end(), sum);
		if (index != complementary.end()) {
			is_found[std::distance(complementary.begin(), index)] = true;
		}
	}

	return !std::any_of(is_found.begin(), is_found.end(), [](auto const& found){return !found;}); // Check if all booleans are true
}

template <typename t> bool is_possible_complete(t n, t size, t max_first_val, t first_val) {
    unsigned long element_set_size = size;

    unsigned long complementary_size = n - element_set_size;

    unsigned long raw_sum_size = (element_set_size * (element_set_size + 1)) >> 1;

    return (raw_sum_size >= complementary_size) && (first_val <= max_first_val);
}

template <typename t> std::vector<std::vector<t>> get_complete_sum_free_set(t n) {
	std::vector<std::vector<t>> out;
	std::vector<t> stack(n+1);
	std::vector<t> sums;
	std::vector<t> tmp;
    long max_first_value = (n+1) / 3;
	long size = 0;
	long index = 0;
    bool current_is_sum_free = true;

	while(true){
		if(stack[size] < n-1) {
			stack[size+1] = stack[size] + 1;
			size++;
		}
		else {
			stack[size-1]++;
			size--;
		}

		if(size == 0) {
            break;
        }

        if (stack[2] == (stack[1] << 1)) {
            continue; // A check to see if the second value is equal to twice the first a.k.a. is b = a + a where b is the second value and a is the first
        }

        if(is_possible_complete(n, size, max_first_value, stack[1])) {
            tmp.resize(size);
            std::copy(stack.begin() + 1, stack.begin() + 1 + size, tmp.begin());
            for (auto i = 1; i < 1 + size && current_is_sum_free; i++) {
                for (auto j = i; j < 1 + size; j++) {
                    long sum = (stack[i] + stack[j]) % n; // Calculate the sum mod n
                    if (vector_contains(tmp, sum)) {
                        current_is_sum_free = false;
                        continue;
                    }
                    sums.push_back(sum);
                }
            }

            if (current_is_sum_free) {
                if (is_complete(tmp, sums, n)) {
                    std::cout << "Complete Sum Free: " << tmp << std::endl;
                    out.emplace_back(size);
                    std::copy(stack.begin() + 1, stack.begin() + 1 + size, out[index].begin());
                    index++;
                }
            }
            sums.clear();
            current_is_sum_free = true;
        }
	}

	return out;
}