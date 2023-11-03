#include <iomanip>
#include <iostream>
#include <limits>
#include <chrono>

// Allow to change the floating point type
using my_float = long double;

my_float pi_taylor(size_t steps) {
	my_float result = 0;
	int sign = -1;
	for(size_t n = 0; n < steps; n++){
		sign = -sign;
		result += sign / static_cast<my_float>(2 * n + 1);
		/*if (n > steps-5) {
			std::cout << result << std::endl;
			std::cout << sign << std::endl;
		}*/
	}
	result = result * 4;
    return result;
}

int main(int argc, const char *argv[]) {

    // read the number of steps from the command line
    if (argc != 2) {
        std::cerr << "Invalid syntax: pi_taylor <steps>" << std::endl;
        exit(1);

    }

    size_t steps = std::stoul(argv[1]);//std::stoll(argv[1]);
    auto start = std::chrono::steady_clock::now();
    auto pi = pi_taylor(steps);
	auto stop = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() == 0) {
		std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::microseconds>
                (stop-start).count() << "us" << std::endl;
	} else {
		std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>
			(stop-start).count() << "ms" << std::endl;
	}
	std::cout << "For " << steps << ", pi value: "
        << std::setprecision(std::numeric_limits<my_float>::digits10 + 1)
        << pi << std::endl;
}
