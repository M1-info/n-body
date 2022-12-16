CCC=mpicxx

CFLAGS=-std=c++17 -Wall -I /urs/local/include -I src/glad/include
LDFLAGS=-L src/glad/src -lGL -lglad -lglfw -lm

DEBUBFLAG=-g

ifeq ($(ARGS),)
		ARGS=no-symmetry
endif

SRC = src/main-$(ARGS).cpp src/utils.cpp

# SRC = $(wildcard src/imgui/*.cpp)
# SRC += $(wildcard src/*.cpp)

OBJ = $(SRC:src/.cpp=.o)
# OBJ += $(SRC:src/include/imgui/.cpp=.o)

BINMAIN = ./bin/
EXEC = N-Body

all: $(EXEC)

$(EXEC): $(OBJ)
		$(CCC) $(DEBUBFLAG) -o $(BINMAIN)$@ $^ $(LDFLAGS)

%.o: %.cpp
		$(CCC) $(DEBUBFLAG) -o $@ -c $< $(CFLAGS)

clean:
		rm -rf *.o

mrproper: clean
		rm -rf $(BINMAIN)$(EXEC)