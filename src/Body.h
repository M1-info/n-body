#ifndef BODY_H
#define BODY_H

#include <glm/glm.hpp>

class Body
{
private:
    glm::vec2 position;
    glm::vec2 velocity;
    float mass;
    float radius;

public:
    Body();
    Body(glm::vec2 position, glm::vec2 velocity, float mass, float radius);
    ~Body();

    glm::vec2 getPosition();
    glm::vec2 getVelocity();
    float getMass();
    float getRadius();

    void setPosition(glm::vec2 position);
    void setVelocity(glm::vec2 velocity);
    void setMass(float mass);
    void setRadius(float radius);
};

#endif // BODY_H