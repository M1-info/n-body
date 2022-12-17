#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <math.h>
#include <fstream>
#include <iostream>

// compute the force between two bodies
void computeForces(double position_current[2], double position_other[2], double mass_current, double mass_other, double *forces);

std::string readShaderSource(const char *shaderFile);

#endif // UTILS_H