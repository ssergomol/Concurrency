from cProfile import label
import struct
import matplotlib.pyplot as plt
import numpy as np

name = "case A (90% read only)"
name1 = "lock_free_case_A"
time1 = []
threads1 = []

with open("./" + name1, "rb") as file:
    valueNumber = 0
    while (bytes := file.read(8)):
        # value = struct.unpack('d', bytes)
        value = int.from_bytes(bytes, byteorder='little')
        if valueNumber % 2 == 0:
            threads1.append(value)
        else:
            time1.append(value / (1e3))
        valueNumber += 1

for point, thread in zip(time1, threads1):
    print(thread, point)

name2 = "lock_based_case_A"
time2 = []
threads2 = []

with open("./" + name2, "rb") as file:
    valueNumber = 0
    while (bytes := file.read(8)):
        # value = struct.unpack('d', bytes)
        value = int.from_bytes(bytes, byteorder='little')
        if valueNumber % 2 == 0:
            threads2.append(value)
        else:
            time2.append(value / (1e3))
        valueNumber += 1

for point, thread in zip(time2, threads2):
    print(thread, point)

fig, ax = plt.subplots()
ax.scatter(threads1, time1, label=name2)
ax.scatter(threads2, time2, label=name1)
ax.set(ylabel='time, millisecs', xlabel='threads_number',
       title= name)
# ax.set_ylim(0, 60)
ax.legend()
ax.grid()

fig.savefig("./plots/" + name + ".png")