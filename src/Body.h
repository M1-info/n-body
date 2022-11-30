#ifndef BODY_H
#define BODY_H

#include <glm/glm.hpp>
#include <vector>
#include <iostream>

class Body
{
private:
    glm::vec2 m_position; // in meters
    glm::vec2 m_velocity;
    glm::vec2 m_forces;
    float m_mass; // in kg
    float m_radius;

public:
    Body();
    Body(glm::vec2 position, glm::vec2 velocity, float mass, float radius);
    ~Body();

    glm::vec2 getPosition() const;
    glm::vec2 getVelocity() const;
    float getMass() const;
    float getRadius() const;
    glm::vec2 getForces() const;

    void setPosition(glm::vec2 position);
    void setVelocity(glm::vec2 velocity);
    void setMass(float mass);
    void setRadius(float radius);
    void setForces(glm::vec2 force);

    void computeForces(const std::vector<Body *> &bodies);

    void debug() const;
};

#endif // BODY_H