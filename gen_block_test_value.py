import numpy as np

np.random.seed(0)

size = 0x10

left = np.random.rand(size, size)
right = np.random.rand(size, size)

dest = left @ right

f = open("./block_test_value", "w")

for per_y in left:
    for element in per_y:
        f.write(f"{element},")
for per_y in right:
    for element in per_y:
        f.write(f"{element},")
for per_y in dest:
    for element in per_y:
        f.write(f"{element},")

f.close()

# print(left)
# print(right)

# print(dest)


