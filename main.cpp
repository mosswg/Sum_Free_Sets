#include <iostream>
#include <vector>

/*
 * to - from
 * ---------
 * \
 *  \
 *   |      i
 *  /
 * /
 * --------
 * i = 1
 */
long sum(long from, long to) {
	long sum = 0;
	for (int i = 1; i <= (to-from); i++) {
		sum += i;
	}
	return sum;
}

void create_powerset(std::vector<int> const& set, std::vector<int>& out, int index, int size) {
	for(int j = 0; j < size-1; j++) {
		for (int i = index; i < set.size(); i++) {

		}
	}
}

std::vector<std::vector<int>> create_powersets_at_index(std::vector<int> const& set, int index) {
	std::vector<std::vector<int>> out(sum(index, (long)set.size())); // Allocate the array with the size calculated by the sum function

	for(int i = 0; i < set.size(); i++) {
		out[index].push_back(set[index]);
		create_powerset(set, out[index], index, i);
	}

	return out;
}


std::vector<std::vector<int>> create_possible_sum_free_sets(int from, int to) {
	std::vector<std::vector<int>> out;
	std::vector<int> set;

	// Create a set of all the values. e.g. from 0 to 5 -> {0, 1, 2, 3, 4}
	for(int i = from; i < to; i++) {
		set.push_back(i);
	}

	// Fill the out vector with all the combinations
	for(int i = from+1; i < to; i++) {
		std::vector<std::vector<int>> index_powerset = create_powersets_at_index(set, i);
		out.insert(out.end(), index_powerset.begin(), index_powerset.end());
	}

	return out;
}

int main() {

	return 0;
}
