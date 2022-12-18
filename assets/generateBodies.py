import random

BODIES_COUNT = 5000
FILE_NAME = "./assets/input.txt"

# clear file
f = open(FILE_NAME, "w+").close()

# generate random bodies and write in a file
for i in range(BODIES_COUNT):
    id = i
    m = random.uniform(1000, 2000)
    x = random.uniform(-5, 5)
    y = random.uniform(-5, 5)
    f = open(FILE_NAME, "a")
    f.write(str(id) + " " + str(m) + " " + str(x) + " " + str(y) + "\n")
f.close()