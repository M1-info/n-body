#ifndef N_BODY_H
#define N_BODY_H

#include <ostream>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>

#include <mpi.h>

#include "utils.h"
// #include "Render.h"

#define GRAVITATIONAL_CONSTANT 6.67408e-11 // m / (kg * s^2)
#define DELTA_T 1                          // in seconds
#define SENDED_DATA_SIZE 2
#define NB_BODY_TOTAL 800
#define NB_ITERATIONS 1000

#define HOST_RANK 0

#define VELOCITY_X_INDEX 0
#define VELOCITY_Y_INDEX 1
#define POSITION_X_INDEX 0
#define POSITION_Y_INDEX 1

#endif // N_BODY_H