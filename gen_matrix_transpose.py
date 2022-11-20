import numpy as np
from load_header import settings

np.random.seed(0)

size = settings["MATRIX_SIZE"]

mat = np.random.rand(size, size)

f = open("./matrix_transpose_value", "w")

for per_y in mat:
    for element in per_y:
        f.write(f"{element},")

for per_y in mat.T:
    for element in per_y:
        f.write(f"{element},")
