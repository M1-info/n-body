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
#define DELTA_T 1           // in seconds
#define SENDED_DATA_SIZE 6
#define NB_BODY_TOTAL 10
#define NB_ITERATIONS 100

#define HOST_RANK 0

#define ID_INDEX 0
#define MASS_INDEX 1
#define VELOCITY_X_INDEX 2
#define VELOCITY_Y_INDEX 3
#define POSITION_X_INDEX 4
#define POSITION_Y_INDEX 5

#endif // N_BODY_H