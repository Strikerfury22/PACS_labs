import matplotlib.pyplot as plt
our_implementation_user= [0.001, 0.002, 0.006, 0.781, 1.599, 3.647, 36.278, 329.159,  3401.547]
eigen_vectorized_user=[0.000, 0.002, 0.003, 0.059, 0.263, 2.263, 2.226, 17.734, 146.548]
eigen_nonvectorized_user=[0.000, 0.002, 0.001, 0.062, 0.119, 0.267, 2.274, 17.738, 141.751]
our_implementation_system= [0.001, 0.001, 0.001, 0.003, 0.002, 0.005, 0.022, 0.039, 0.361]
eigen_vectorized_system=[0.002, 0.000, 0.000, 0.005, 0.007, 0.007, 0.014, 0.017, 0.0167]
eigen_nonvectorized_system=[0.002, 0.000, 0.001, 0.003, 0.002, 0.003, 0.006, 0.024, 0.081]
y_axis = [1,2,3,4,5,6,7,8,9]
plt.plot(y_axis, our_implementation_user, 'r', label = 'Our implementation')
plt.plot(y_axis, eigen_vectorized_user, 'b', label = 'Eigen Vectorized')
plt.plot(y_axis, eigen_nonvectorized_user, 'g', label = 'Eigen Non-Vectorized')
plt.ylabel('Time (seconds)')
plt.xlabel('Test Number')
plt.title('User Times')
plt.legend()
plt.show()

plt.plot(y_axis, our_implementation_system, 'r', label = 'Our implementation')
plt.plot(y_axis, eigen_vectorized_system, 'b', label = 'Eigen Vectorized')
plt.plot(y_axis, eigen_nonvectorized_system, 'g', label = 'Eigen Non-Vectorized')
plt.ylabel('Time (seconds)')
plt.xlabel('Test Number')
plt.title('System Times')
plt.legend()
plt.show()

#our_implementation = [1, 2, 5, 55]
#plt.plot(our_implementation)
#plt.show
