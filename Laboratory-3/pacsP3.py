import matplotlib.pyplot as plt
test1= [343219,175376, 89238, 44377, 22378]
test2=[343225,175952, 89427, 44376, 22215]
test3=[343210,175382, 89423, 44374, 22213]
mean = [343218, 175570, 88696, 44375, 22268]
standard_deviation = [7.549834435, 330.8353065, 469.3900297, 1.527525232, 94.69072464]
CV = [2.19972E-05, 0.00188435, 0.005292122, 3.44226E-05, 0.004252196]
x_axis = [1, 2, 4, 8, 16]


plt.plot(x_axis, mean, 'r')
plt.ylabel('Time (seconds)')
plt.xlabel('Number of Threads')
plt.title('Mean')
plt.show()

plt.plot(x_axis, standard_deviation, 'g')
plt.ylabel('Deviation')
plt.xlabel('Number of Threads')
plt.title('Standard deviation')
plt.show()

plt.plot(x_axis, CV, 'b', label = 'Coefficient of Variation')
plt.ylabel('Coefficient')
plt.xlabel('Number of Threads')
plt.title('Coefficient of Variation')
plt.show()

plt.plot(x_axis, test1, 'r', label = 'Test1')
plt.plot(x_axis, test2, 'b', label = 'Test2')
plt.plot(x_axis, test3, 'g', label = 'Test3')
plt.ylabel('Time (milliseconds)')
plt.xlabel('Number of Threads')
plt.title('4294967295 step Testing times')
plt.legend()
plt.show()

#our_implementation = [1, 2, 5, 55]
#plt.plot(our_implementation)
#plt.show
