#include "utils.h"

void computeForces(double position_current[2], double position_other[2], double mass_current, double mass_other, double *forces)
{
    double dx = position_current[0] - position_other[0];
    double dy = position_current[1] - position_other[1];
    double norm = sqrt(dx * dx + dy * dy);

    forces[0] += (mass_current * mass_other * dx) / (norm * norm * norm);
    forces[1] += (mass_current * mass_other * dy) / (norm * norm * norm);
}

std::string readShaderSource(const char *shaderFile)
{
    std::string content;
    std::ifstream fileStream(shaderFile, std::ios::in);

    if (!fileStream.is_open())
    {
        std::cerr << "Could not read file " << shaderFile << ". File does not exist." << std::endl;
        return "";
    }

    std::string line = "";
    while (!fileStream.eof())
    {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}