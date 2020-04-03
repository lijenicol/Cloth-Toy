#include "PhysicsTriangle.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

PhysicsTriangle::PhysicsTriangle(Particle* p1, Particle* p2, Particle* p3) {
	this->particles[0] = p1;
	this->particles[1] = p2;
	this->particles[2] = p3;
	normal = calculateNormal();
}

glm::vec3 PhysicsTriangle::calculateNormal() {
	return glm::normalize(glm::cross(particles[1]->getPosition() - particles[0]->getPosition(), 
		particles[2]->getPosition() - particles[0]->getPosition()));
}

float PhysicsTriangle::calculateArea() {
	return 0.5 * glm::length(glm::cross(particles[1]->getPosition() - particles[0]->getPosition(),
		particles[2]->getPosition() - particles[0]->getPosition()));
}

glm::vec3 PhysicsTriangle::calculateVelocity() {
	return (particles[0]->getVelocity() + particles[1]->getVelocity() + particles[2]->getVelocity()) / 3.f;
}

void PhysicsTriangle::applyAerodynamicForce(glm::vec3 wind) {
	//calculate relative velocity
	glm::vec3 relativeVelocity = calculateVelocity() - wind;
	if (relativeVelocity == glm::vec3(0)) {
		return;
	}

	//calculate cross sectional area
	float crossArea = calculateArea() * (glm::dot(glm::normalize(relativeVelocity), normal));

	//calculate force
	glm::vec3 force = -0.5f * 10.225f * glm::pow(glm::length(relativeVelocity), 2) * 1.75f * crossArea * normal;

	//apply force to particles
	/*particles[0]->applyForce(force / 3.f);
	particles[1]->applyForce(force / 3.f);
	particles[2]->applyForce(force / 3.f);*/

	particles[0]->applyForce(force);
	particles[1]->applyForce(force);
	particles[2]->applyForce(force);
}

void PhysicsTriangle::update() {
	normal = calculateNormal();
}

void PhysicsTriangle::addNormalToParticles() {
	particles[0]->addNormal(normal);
	particles[1]->addNormal(normal);
	particles[2]->addNormal(normal);
}

Particle* PhysicsTriangle::intersectedParticle(glm::vec3 rayDirection, glm::vec3 rayPoint) {
	//return if ray is parallel with triangle
	if (glm::dot(normal, rayDirection) == 0)
		return NULL;

	//calculate plane intersection
	float d = glm::dot(normal, particles[0]->getPosition());
	float t = (d - glm::dot(normal, rayPoint)) / glm::dot(normal, rayDirection);
	glm::vec3 intersectionPoint = rayPoint + t * rayDirection;

	//calculate if intersection point is within triangle
	float testOne = glm::dot(glm::cross(particles[1]->getPosition() - particles[0]->getPosition(), 
		intersectionPoint - particles[0]->getPosition()),
		normal);
	if (testOne < 0)
		return NULL;

	float testTwo = glm::dot(glm::cross(particles[2]->getPosition() - particles[1]->getPosition(),
		intersectionPoint - particles[1]->getPosition()),
		normal);
	if (testTwo < 0)
		return NULL;

	float testThree = glm::dot(glm::cross(particles[0]->getPosition() - particles[2]->getPosition(),
		intersectionPoint - particles[2]->getPosition()),
		normal);
	if (testThree < 0)
		return NULL;

	//return the first particle for time being
	return particles[0];
}