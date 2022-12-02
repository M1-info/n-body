#include "utils.h"

NBodyType randMinmax(NBodyType min, NBodyType max)
{    
    std::uniform_real_distribution<NBodyType> dist(min, max);
    //Mersenne Twister: Good quality random number generator
    std::mt19937 rng; 
    //Initialize with non-deterministic seeds
    rng.seed(std::random_device{}());

    return dist(rng);
}