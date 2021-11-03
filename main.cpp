#include <iostream>
#include <algorithm>
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

long long number_powersets(long long n) {
	return 1 << n;
}

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


// From: https://www.programminglogic.com/powerset-algorithm-in-c/
template <typename t> void create_powersets(t n, std::vector<std::vector<t>>& out){
	std::vector<t> stack(10);
	long size = 0;
	long index = 0;

	while(true){
		if (stack[size]<n){
			stack[size+1] = stack[size] + 1;
			size++;
		}
		else{
			stack[size-1]++;
			size--;
		}

		if (size == 0)
			break;

		out.emplace_back(size);
		std::copy(stack.begin()+1, stack.begin()+1+size, out[index].begin());

		index++;
	}
}

template <typename t> bool vector_contains(std::vector<t> const& vec, t const& search) {
	return std::any_of(vec.begin(), vec.end(), [search](auto const& element){return element == search;});
}

template <typename t> bool is_sum_free(std::vector<t> const& set, std::vector<t> const& sums) {
	return !std::any_of(sums.begin(), sums.end(), [set](auto const& sum){ return vector_contains(set, sum); });
}

template <typename t> bool is_complete(std::vector<t> const& set, std::vector<t> const& sums) {
	std::vector<bool> is_found(set.size());
	for(auto const& sum : sums) {
		auto index = std::find(set.begin(), set.end(), sum);
		if (index != set.end()) {
			is_found[std::distance(set.begin(), index)] = true;
		}
	}
	return !std::any_of(is_found.begin(), is_found.end(), [](auto const& found){return !found;}); // Check if all booleans are true
}

template <typename t> std::vector<t> complementary_set(std::vector<t> const& set, t n) {
	std::vector<t> out;

	for(t i = 0; i < n; i++) {
		if (!vector_contains(set, i)) {
			out.push_back(i);
		}
	}

	return out;
}

template <typename t> bool is_complete_sum_free_set(std::vector<t> const& set, t n) {
	std::vector<t> sums;

	for(long long i = 0; i < set.size(); i++) {
		for(long long j = i; j < set.size(); j++) {
			sums.push_back((set[i] + set[j]) % n); // Calculate the sum mod n
		}
	}

	if (is_sum_free(set, sums)) {
		std::vector<t> complementary = complementary_set(set, n);
		return is_complete(complementary, sums);
	}
	return false;


}


int main() {
	long long n = 11;

	std::vector<std::vector<long long>> power_sets;

	create_powersets(n, power_sets);

	std::vector<std::vector<long long>> complete_sum_free_sets;

	for (auto const& power_set : power_sets) {
		if (is_complete_sum_free_set(power_set, n)) {
			complete_sum_free_sets.push_back(power_set);
		}
	}

	std::cout << complete_sum_free_sets << std::endl;

	return 0;
}
