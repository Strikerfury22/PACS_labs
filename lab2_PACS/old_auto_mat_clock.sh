g++ -O2 -Wall -I eigen-3.4.0/ matrix_mult_test_clock.cpp -o mat_clock

echo 'Test 1200x1200'
time ./mat_clock 6

time ./mat_clock 6

time ./mat_clock 6

time ./mat_clock 6

echo 'Test 2500x2500'
time ./mat_clock 7

time ./mat_clock 7

time ./mat_clock 7

time ./mat_clock 7

echo 'Test 5000x5000'
time ./mat_clock 8

time ./mat_clock 8

time ./mat_clock 8

time ./mat_clock 8