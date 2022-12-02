#ifndef BODY_H
#define BODY_H

#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <math.h>

typedef double NBodyType;
typedef glm::dvec2 Vec2;

class Body
{
private:
    NBodyType m_id;
    Vec2 m_position; // in meters
    Vec2 m_velocity; // in meters by second ??
    Vec2 m_forces;
    NBodyType m_mass; // in kg
    NBodyType m_radius;

public:
    Body();
    Body(Vec2 position, Vec2 velocity, NBodyType mass, NBodyType radius);
    ~Body();

    NBodyType getId() const;
    Vec2 getPosition() const;
    Vec2 getVelocity() const;
    NBodyType getMass() const;
    NBodyType getRadius() const;
    Vec2 getForces() const;

    void setPosition(Vec2 position);
    void setVelocity(Vec2 velocity);
    void setMass(NBodyType mass);
    void setRadius(NBodyType radius);
    void setForces(Vec2 force);

    Vec2 computeForces(NBodyType mass, Vec2 position);
    void computePosition(NBodyType delta_time);
    void computeVelocity(NBodyType delta_time);

    void debug() const;
};

#endif // BODY_H