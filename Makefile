CCC=mpicxx

CFLAGS=-std=c++17 -Wall -I /urs/local/include
LDFLAGS= -lGL -lglfw -lm

DEBUBFLAG=-g

SRC = $(wildcard src/imgui/*.cpp)
SRC += $(wildcard src/*.cpp)
OBJ = $(SRC:src/include/imgui/.cpp=.o)
OBJ += $(SRC:src/.cpp=.o)
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