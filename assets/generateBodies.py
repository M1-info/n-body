import random

BODIES_COUNT = 5000
FILE_NAME = "./assets/bodies.txt"

# clear file
f = open(FILE_NAME, "w+").close()

# generate random bodies and write in a file
for i in range(BODIES_COUNT):
    id = random.uniform(0, 1000000000)
    m = random.uniform(0, 10)
    vx = random.uniform(-10, 10)
    vy = random.uniform(-10, 10)
    x = random.uniform(0, 100)
    y = random.uniform(0, 100)
    f = open(FILE_NAME, "a")
    f.write(str(id) + " " + str(m) + " " + str(vx) + " " + str(vy) + " " + str(x) + " " + str(y) + "\n")
f.close()