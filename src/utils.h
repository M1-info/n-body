#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include <cstdlib>
#include <random>
#include <math.h>
#include <iostream>


double randMinmax(double min, double max);

// compute the force between two bodies
void computeForces(double position_current[2], double position_other[2], double mass_current, double mass_other, double *forces);


#endif // UTILS_H