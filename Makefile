CCC=mpicxx

CFLAGS=-std=c++17 -Wall -I /urs/local/include -I src/glad/include
LDFLAGS=-L src/glad/src -lGL -lglad -lglfw -lm

DEBUBFLAG=-g

SRC = src/main.cpp src/Body.cpp

# SRC = $(wildcard src/imgui/*.cpp)
# SRC += $(wildcard src/*.cpp)

OBJ = $(SRC:src/.cpp=.o)
# OBJ += $(SRC:src/include/imgui/.cpp=.o)

BINMAIN = ./bin/
BINOBJ = ./obj/
EXEC = N-Body

all: $(EXEC)

$(EXEC): $(OBJ)
		$(CCC) -std=c++17 $(DEBUBFLAG) -o $(BINMAIN)$@ $^ $(LDFLAGS)

%.o: %.cpp
		$(CCC) -std=c++17 $(DEBUBFLAG) -o $@ -c $< $(CFLAGS)

clean:
		rm -rf *.o

mrproper: clean
		rm -rf $(BINMAIN)$(EXEC)
