#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <math.h>
#include <iostream>
#include <fstream>

// compute the force exerted by the body other on the body current
// forces: array of size 2 containing the force on x and y (return parameter)
void computeForces(double position_current[2], double position_other[2], double mass_current, double mass_other, double *forces);

#ifdef VISUALISATION
// read the content of a file and return it as a string
std::string readShaderSource(const char *shaderFile);
#endif

#endif // UTILS_H