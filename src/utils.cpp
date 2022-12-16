#include "utils.h"

double randMinmax(double min, double max)
{    
    std::uniform_real_distribution<double> dist(min, max);
    //Mersenne Twister: Good quality random number generator
    std::mt19937 rng; 
    //Initialize with non-deterministic seeds
    rng.seed(std::random_device{}());

    return dist(rng);
}

void computeForces(double position_current[2], double position_other[2], double mass_current, double mass_other, double *forces)
{
    double dx = position_current[0] - position_other[0];
    double dy = position_current[1] - position_other[1];
    double norm = sqrt(dx * dx + dy * dy);
    forces[0] += (mass_current * mass_other * dx) / (norm * norm * norm);
    forces[1] += (mass_current * mass_other * dy) / (norm * norm * norm);
}
