#ifndef N_BODY_H
#define N_BODY_H

#include <ostream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include <mpi.h>

#include "utils.h"

#ifdef VISUALISATION
#include "Render.h"
#endif

#define GRAVITATIONAL_CONSTANT 6.67408e-11 // m / (kg * s^2)
#define DELTA_T 1
#define SENDED_DATA_SIZE 2
#define NB_BODY_TOTAL 800
#define NB_ITERATIONS 2000

#define HOST_RANK 0

#define VELOCITY_X_INDEX 0
#define VELOCITY_Y_INDEX 1
#define POSITION_X_INDEX 0
#define POSITION_Y_INDEX 1
#define FORCE_X_INDEX 0
#define FORCE_Y_INDEX 1

#define INPUT_FILE "./assets/input.txt"
#define OUTPUT_FILE "./assets/output.txt"

#define INPUT_FILE "./assets/input.txt"
#define OUTPUT_FILE "./assets/output.txt"

#endif // N_BODY_H