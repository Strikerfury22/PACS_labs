import matplotlib.pyplot as plt
best10= [2394.8,2445.2,2491.8,2512.2,2518.4,2521.6,2523.4,2609.6,2616.8,2627]
worst10=[16320.2,9340.4,9240.4,6813.6,5824.8,5646.6,5139.2,4332.2,4119.2,3961, 2394.8]
CV = [2.19972E-05, 0.00188435, 0.005292122, 3.44226E-05, 0.004252196]
x_axis_best10 = ["1 48", "1 24", "2 16", "4 8", "16 2", "8 4", "1 32", "32 1", "2 12", "1 12"]
x_axis_worst10 = ["1 1","2 1","1 2","1 3", "2 2","4 1","1 4","2 3","1 6","4 2", "1 48(best)"]

plt.plot(x_axis_best10, best10, 'g')
plt.ylabel('Time (milliseconds)')
plt.xlabel('nº of width and height partitions in tasks')
plt.title('Rendering time (12 thread machine)')
plt.show()
plt.plot(x_axis_worst10[:10], worst10[:10], 'r')
plt.plot(x_axis_worst10[9:], worst10[9:], 'g+')
plt.plot(x_axis_worst10[9:], worst10[9:], 'g')
#plt.plot(x_axis_worst10, worst10, 'g', 2394.8, "1 48", 'r+')
#plt.plot(2394.8, "1 48", 'r+')
plt.ylabel('Time (milliseconds)')
plt.xlabel('nº of width and height partitions in tasks')
plt.title('Rendering time (12 thread machine)')
plt.show()
