#ifndef N_BODY_H
#define N_BODY_H

#include <ostream>
#include <iostream>
#include <iterator>
#include <vector>
#include <cstdlib>
#include <time.h>
// #include <thread>
#include <chrono>
#include <unistd.h>

#include <mpi.h>

#include "Body.h"
#include "utils.h"
// #include "Render.h"

#define GRAVITY_CONSTANT 6.67408e-11 // m / (kg * s^2)
#define TIME_STEP 0.0001             // in seconds
#define DATA_SIZE 4
#define NB_BODY_TOTAL 800
#define NB_ITERATIONS 1000

#endif // N_BODY_H