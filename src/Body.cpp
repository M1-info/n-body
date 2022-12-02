#include "Body.h"

Body::Body() : m_position(Vec2(0.0f)), m_velocity(Vec2(0.0f)), m_mass(0.0f), m_radius(0.0f), m_forces(Vec2(0.0f))
{
    m_id = randMinmax(1, 999999999);
}

Body::Body(Vec2 position, Vec2 velocity, NBodyType mass, NBodyType radius)
    : m_position(position), m_velocity(velocity), m_mass(mass), m_radius(radius), m_forces(Vec2(0.0f))
{
    m_id = randMinmax(1, 999999999);
}

Body::~Body()
{
}

NBodyType Body::getId() const
{
    return m_id;
}

Vec2 Body::getPosition() const
{
    return m_position;
}

Vec2 Body::getVelocity() const
{
    return m_velocity;
}

NBodyType Body::getMass() const
{
    return m_mass;
}

NBodyType Body::getRadius() const
{
    return m_radius;
}

Vec2 Body::getForces() const
{
    return m_forces;
}

void Body::setPosition(Vec2 position)
{
    m_position = position;
}

void Body::setVelocity(Vec2 velocity)
{
    m_velocity = velocity;
}

void Body::setMass(NBodyType mass)
{
    m_mass = mass;
}

void Body::setRadius(NBodyType radius)
{
    m_radius = radius;
}

void Body::setForces(Vec2 forces)
{
    m_forces = forces;
}

Vec2 Body::computeForces(NBodyType mass, Vec2 position)
{
    // TODO: si position < masse -> le plus gros bouffe l'autre
    Vec2 distance = m_position - position;
    NBodyType norm = sqrt(distance.x * distance.x + distance.y * distance.y);

    Vec2 res = distance * (mass / pow(norm, 3));
    //std::cout << "computeForces(" << mass << ", [" << position.x << ", " << position.y << "]) with m_pos [" << m_position.x << ", " << m_position.y << "] = [" << res.x << ", " << res.y << "]      (distance = [" << distance.x << ", " << distance.y << "] - norm = " << norm << ") " << std::endl;
    return res;
}

void Body::computePosition(NBodyType delta_time)
{
    m_position += delta_time * m_velocity;
}

void Body::computeVelocity(NBodyType delta_time)
{
    m_velocity += delta_time * m_forces / m_mass;
    //std::cout << "computeVelocity with delta_time=" << delta_time << ", m_forces=[" << m_forces.x << ", " << m_forces.y << "] and m_mass =" << m_mass << "   =    [" << m_velocity.x << ", " << m_velocity.y << "] " << std::endl;
}

void Body::debug() const
{
    std::cout << "Body id-" << m_id << std::endl;
    std::cout << "Position: " << m_position.x << ", " << m_position.y << std::endl;
    std::cout << "Velocity: " << m_velocity.x << ", " << m_velocity.y << std::endl;
    std::cout << "Mass: " << m_mass << std::endl;
    std::cout << "Radius: " << m_radius << std::endl;
    std::cout << std::endl;
}