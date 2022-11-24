#include "Body.h"

Body::Body() : position(glm::vec2(0.0f)), velocity(glm::vec2(0.0f)), mass(0.0f), radius(0.0f)
{
}

Body::Body(glm::vec2 position, glm::vec2 velocity, float mass, float radius)
    : position(position), velocity(velocity), mass(mass), radius(radius)
{
}

Body::~Body()
{
}

glm::vec2 Body::getPosition()
{
    return position;
}

glm::vec2 Body::getVelocity()
{
    return velocity;
}

float Body::getMass()
{
    return mass;
}

float Body::getRadius()
{
    return radius;
}

void Body::setPosition(glm::vec2 position)
{
    this->position = position;
}

void Body::setVelocity(glm::vec2 velocity)
{
    this->velocity = velocity;
}

void Body::setMass(float mass)
{
    this->mass = mass;
}

void Body::setRadius(float radius)
{
    this->radius = radius;
}