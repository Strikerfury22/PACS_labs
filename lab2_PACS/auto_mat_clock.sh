g++ -O2 -Wall matrix_mult_test_clock.cpp -o mat_clock
readonly ITERATIONS=5

echo "Test 1200x1200"
for i in $(seq 1 $ITERATIONS)
do
	echo "---------------------"
	echo "test nº $i"
	time ./mat_clock 6
done

echo ""
echo "Test 2500x2500"
for i in $(seq 1 $ITERATIONS)
do
	echo "---------------------"
	echo "test nº $i"
	time ./mat_clock 7
done

echo ""
echo "Test 5000x5000"
for i in $(seq 1 $ITERATIONS)
do
	echo "---------------------"
	echo "test nº $i"
	time ./mat_clock 8
done