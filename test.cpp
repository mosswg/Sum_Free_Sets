//
// Copyright (c) 2021 Moss Gallagher.
//
#include "complete_sum_free.h"

int main() {
	const long lim = 25;


	for (long n = 2; n <= lim; n++) {
		std::cout << "\nn=" << n << '\n';
		get_complete_sum_free_set(n);
	}

	return 0;
}
