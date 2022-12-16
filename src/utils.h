#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include <cstdlib>
#include <random>
#include <math.h>
#include <iostream>


double randMinmax(double min, double max);

// compute the force exerted by the body other on the body current
// forces: array of size 2 containing the force on x and y (return parameter)
void computeForces(double position_current[2], double position_other[2], double mass_current, double mass_other, double *forces);


#endif // UTILS_H