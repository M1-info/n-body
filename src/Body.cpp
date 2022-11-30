#include "Body.h"


Body::Body() : m_position(glm::vec2(0.0f)), m_velocity(glm::vec2(0.0f)), m_mass(0.0f), m_radius(0.0f), m_forces(glm::vec2(0.0f))
{
}

Body::Body(glm::vec2 position, glm::vec2 velocity, float mass, float radius)
    : m_position(position), m_velocity(velocity), m_mass(mass), m_radius(radius), m_forces(glm::vec2(0.0f))
{
}

Body::~Body()
{
}

glm::vec2 Body::getPosition() const
{
    return m_position;
}

glm::vec2 Body::getVelocity() const
{
    return m_velocity;
}

float Body::getMass() const
{
    return m_mass;
}

float Body::getRadius() const
{
    return m_radius;
}

glm::vec2 Body::getForces() const
{
    return m_forces;
}

void Body::setPosition(glm::vec2 position)
{
    m_position = position;
}

void Body::setVelocity(glm::vec2 velocity)
{
    m_velocity = velocity;
}

void Body::setMass(float mass)
{
    m_mass = mass;
}

void Body::setRadius(float radius)
{
    m_radius = radius;
}

void Body::setForces(glm::vec2 forces)
{
    m_forces = forces;
}


void Body::computeForces(const std::vector<Body*> &bodies){
    glm::vec2 forces = glm::vec2(0.0f);
    
    for (Body *body : bodies){
        glm::vec2 distance = this->getPosition() - body->getPosition();
    }
}