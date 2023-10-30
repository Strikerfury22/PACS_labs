#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using my_float = long double;

typedef struct {
    size_t big_chunk;
    size_t regular_chunk;
    size_t last_chunk;
} chunk_info;

/*This function must receive the vector output

*/
void
pi_taylor_chunk(std::vector<my_float> &output,
        size_t thread_id, size_t start_step, size_t stop_step) {
        	my_float result = 0;
	int sign = -1;
	for(size_t start_step; start_step < stop_step; start_step++){
		sign = -sign;
		result += sign / static_cast<my_float>(2 * start_step + 1);
		if (start_step > stop_step-5) {
			std::cout << result << std::endl;
			std::cout << sign << std::endl;
		}
	}
	result = result * 4;
    output[thread_id] = result;

}
/* Splits the number of steps depending on how many threads we declare
For a given number of steps and num_threads, the iterations are divided:
steps % threads (module) receive a total of [(steps / num_threads) + 1] total steps to process
the rest receive [step / num_threads] total steps to process
*/
chunk_info
split_evenly(size_t steps, size_t num_threads)
{
    return {steps / num_threads + 1, steps / num_threads, steps % num_threads};
}
/* Given a previously created struct of type chunk_info,
   Return the correct begin step and end step for a given thread
*/
std::pair<size_t, size_t>
get_chunk_begin_end(const chunk_info& ci, size_t index)
{
    size_t begin = 0, end = 0;
    if (index < ci.last_chunk ) {
        begin = index*ci.big_chunk;
        end = begin + ci.big_chunk; // (index + 1) * ci.large_chunk
    } else {
        begin = ci.last_chunk*ci.big_chunk + (index - ci.last_chunk) * ci.regular_chunk;
        end = begin + ci.regular_chunk;
    }
    return std::make_pair(begin, end);
}

/* This function dictates how to launch the executable with the correct arguments */
std::pair<size_t, size_t>
usage(int argc, const char *argv[]) {
    // read the number of steps from the command line
    if (argc != 3) {
        std::cerr << "Invalid syntax: pi_taylor <steps> <threads>" << std::endl;
        exit(1);
    }

    size_t steps = std::stoll(argv[1]);
    size_t threads = std::stoll(argv[2]);

    if (steps < threads ){
        std::cerr << "The number of steps should be larger than the number of threads" << std::endl;
        exit(1);

    }
    return std::make_pair(steps, threads);
}

int main(int argc, const char *argv[]) {


    auto ret_pair = usage(argc, argv);
    auto steps = ret_pair.first;
    auto num_threads = ret_pair.second;

    //Security
    size_t max_threads = std::min(48u, std::thread::hardware_concurrency());
	if (num_threads > max_threads) {
        num_threads = max_threads;
    }
    my_float pi;
    std::vector<my_float> results(num_threads); //Initialize a vector with as many elements as threads 
    //Split the steps evenly
    auto chunks = split_evenly(steps, num_threads);
	// Here goes calling the function "pi_taylor_chunk" to process and retrieve the result of each thread
    //...
    for (size_t i = 0; i < num_threads; ++i){
        auto begin_end = get_chunk_begin_end(chunks, i);
        thread_vector.
    }
    //Join all vectors and calculate the result

    pi = std::accumulate(results.begin(), results.end(),
                                decltype(results)::value_type(0));
    //Print solution
    std::cout << "For " << steps << ", pi value: "
        << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
        << pi << std::endl;
}

