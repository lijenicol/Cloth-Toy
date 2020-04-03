#include "SpringDamper.h"
#include <iostream>

SpringDamper::SpringDamper(float springConstant, float dampingConstant, float restLength, Particle* particleOne, Particle* particleTwo) {
	this->connectedParticles[0] = particleOne;
	this->connectedParticles[1] = particleTwo;
	this->springConstant = springConstant;
	this->dampingConstant = dampingConstant;
	this->restLength = restLength;
}

SpringDamper::SpringDamper(float springConstant, float dampingConstant, Particle* particleOne, Particle* particleTwo) {
	this->connectedParticles[0] = particleOne;
	this->connectedParticles[1] = particleTwo;
	this->springConstant = springConstant;
	this->dampingConstant = dampingConstant;
	restLength = glm::length(particleTwo->getPosition() - particleOne->getPosition());
}

void SpringDamper::calculateParticleForce() {
	//calculate length
	float length = glm::length(connectedParticles[0]->getPosition() - connectedParticles[1]->getPosition());
	float springForce = -springConstant * (restLength - length);

	//calculate force directions for particles
	glm::vec3 particleOneForceDirection = glm::normalize(connectedParticles[1]->getPosition() - connectedParticles[0]->getPosition());
	
	//calculate velocities in one dimension
	float velocityOne = glm::dot(particleOneForceDirection, connectedParticles[0]->getVelocity());
	float velocityTwo = glm::dot(particleOneForceDirection, connectedParticles[1]->getVelocity());

	//combine into three dimensions and apply to connected particles
	float totalForce = springForce - dampingConstant * (velocityOne - velocityTwo);
	connectedParticles[0]->applyForce(totalForce * particleOneForceDirection);
	connectedParticles[1]->applyForce(-totalForce * particleOneForceDirection);
}

void SpringDamper::update() {
	//update forces on connected particles
	calculateParticleForce();

	//TODO: implement plastic deformation and destruction of spring
}