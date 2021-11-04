//
// Copyright (c) 2021 Moss Gallagher.
//
#include "complete_sum_free.h"
#include <fstream>

int main(int argc, char* argv[]) {
    char *p;
    long lim;
    if (argc == 1) {
        lim = 25;
    }
    else {
        lim = strtol(argv[1], &p, 10);
    }

	std::string filename = "out/complete_sum_free_to_";

    filename += std::to_string(lim);
    filename += ".txt";

    std::ofstream out_file(filename);

    std::cout << "\nn=" << lim << '\n';
    out_file << get_complete_sum_free_set(lim) << "\n\n";

    return 0;

	for (long n = 2; n <= lim; n++) {
        std::cout << "\nn=" << n << '\n';
		out_file << "n=" << n << "\n";
		out_file << get_complete_sum_free_set(n) << "\n\n";

	}

    out_file.close();

	return 0;
}
