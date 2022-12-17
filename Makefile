CCC=mpicxx

CFLAGS=-std=c++17 -Wall -I /urs/local/include
LDFLAGS=

DEBUBFLAG=-g
DEFINE=
HAS_SYMMETRY=

ifeq ($(SYMMETRY),true)
		HAS_SYMMETRY=symmetry
else
		HAS_SYMMETRY=no-symmetry
endif


SRC = src/main-$(HAS_SYMMETRY).cpp src/utils.cpp

ifeq ($(VISUALISATION),true)
		DEFINE += -DVISUALISATION=true
		CFLAGS += -I src/glad/include
		LDFLAGS += -L src/glad/src -lGL -lglad -lglfw -lm
		SRC += src/Render.cpp
endif


OBJ = $(SRC:src/.cpp=.o)

BINMAIN = ./bin/
EXEC = N-Body

all: $(EXEC)

$(EXEC): $(OBJ)
		$(CCC) $(DEBUBFLAG) $(DEFINE) -o $(BINMAIN)$@ $^ $(LDFLAGS)

%.o: %.cpp
		$(CCC) $(DEBUBFLAG) $(DEFINE) -o $@ -c $< $(CFLAGS)

clean-obj:
		rm -rf bin/*.o

clean-all: clean-obj
		rm -rf $(BINMAIN)$(EXEC)