import numpy as np
import time

x = np.random.rand(0x1000, 0x1000)
y = np.random.rand(0x1000, 0x1000)

start = time.perf_counter()
unsed = x @ y
end = time.perf_counter()

print(end - start)

