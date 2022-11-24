CCC=mpicxx

IMGUI_DIR = include/imgui

CFLAGS=-std=c++17 -Wall -I /urs/local/include #-I $(IMGUI_DIR)
LDFLAGS= -lGL -lglfw -lm
#LDFLAGS= -L $(IMGUI_DIR) -lGL -lglfw -lm -limgui 

DEBUBFLAG=-g

SRC = $(wildcard src/*.cpp)
#SRC += $(wildcard $(IMGUI_DIR)/*.cpp)
OBJ = $(SRC:src/.cpp=.o)
BINMAIN = ./bin/
#BINOBJ = ./obj/
EXEC = N-Body

all: $(EXEC)

$(EXEC): $(OBJ)
		$(CCC) $(DEBUBFLAG) -o $(BINMAIN)$@ $^ $(LDFLAGS)

%.o: %.cpp
		$(CCC) $(DEBUBFLAG) -o $@ -c $< $(CFLAGS)

# %.o: $(IMGUI_DIR)/%.cpp
# 	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
		rm -rf *.o

mrproper: clean
		rm -rf $(BINMAIN)$(EXEC)