import matplotlib.pyplot as plt
import numpy as np


time = []
threads = []

with open("./output", "rb") as file:
    valueNumber = 0
    while (bytes := file.read(8)):
        value = int.from_bytes(bytes, byteorder='little')
        if valueNumber % 2 == 0:
            threads.append(value)
        else:
            time.append(value / 1e6)
        valueNumber += 1

for point, thread in zip(time, threads):
    print(thread, point)

fig, ax = plt.subplots()
ax.scatter(threads, time)
ax.set(ylabel='time, sec', xlabel='matrix size',
       title='Concurrent tiling multiplication (matrix size = 20 MB')
ax.grid()

fig.savefig("../plots/output.png")

