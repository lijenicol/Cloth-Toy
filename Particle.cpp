#include "Particle.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

Particle::Particle(float mass, float size, float elasticity, float lifeTime, glm::vec3 position, glm::vec3 velocity, bool fixed) {
	this->mass = mass;
	this->size = size;
	this->elasticity = elasticity;
	this->lifeTime = lifeTime;
	this->position = position;
	this->velocity = velocity;
	this->fixed = fixed;

	currentLife = 0;
	force = glm::vec3(0);
	acceleration = glm::vec3(0);
	normal = glm::vec3(0);
}

void Particle::update(float deltaTime) {
	if (!fixed) {
		//update the position of the particle
		calculatePosition(deltaTime);
	}

	//zero out the force to be added next frame
	force = glm::vec3(0);
	acceleration = glm::vec3(0);
}

glm::vec3 Particle::calculateAcceleration() {
	return force / mass;
}

glm::vec3 Particle::calculateVelocity(float deltaTime) {
	return velocity + acceleration * deltaTime;
}

void Particle::calculatePosition(float deltaTime) {
	//Get number of iterations to loop through per frame
	int targetIter = 4000;
	int iterCount = deltaTime / (1 / (float)targetIter);
	if (iterCount <= 0) {
		iterCount = 1;
	}

	//calculate acceleration and velocity
	acceleration += calculateAcceleration();
	velocity = calculateVelocity(deltaTime / iterCount);

	//update the position of the particle
	for (int i = 0; i < iterCount; i++) {
		position += velocity * (deltaTime / iterCount) + acceleration * glm::pow((deltaTime / iterCount), 2);
		
		//handle collision with ground plane
		if (position.y < 0) {
			position.y = -position.y + 0.001f;
			velocity.y = -elasticity * velocity.y;

			float friction = 0.2f;
			velocity.x = (1 - friction) * velocity.x;
			velocity.z = (1 - friction) * velocity.z;
		}
	}
}

void Particle::applyForce(glm::vec3 force) { this->force += force; }
void Particle::applyAcceleration(glm::vec3 acceleration) { this->acceleration += acceleration; }

void Particle::addNormal(glm::vec3 normal) { this->normal += normal; }
void Particle::setNormal(glm::vec3 normal) { this->normal = normal; }
glm::vec3 Particle::getNormal() { return normal; }

glm::vec3 Particle::getPosition() { return position; }
glm::vec3 Particle::getVelocity() { return velocity; }
glm::vec3 Particle::getForce() { return force; }

bool Particle::getFixed() { return fixed; }
void Particle::setFixed(bool fixed) {this->fixed = fixed;}

void Particle::movePosition(glm::vec3 deltaPosition) { position += deltaPosition; }
void Particle::setPosition(glm::vec3 position) {this->position = position;}
void Particle::setVelocity(glm::vec3 velocity) { this->velocity = velocity; }