#pragma once
#include <glm/glm.hpp>
#include "Particle.h"

class PhysicsTriangle
{
private:
	Particle* particles[3];
	glm::vec3 normal;
public:
	PhysicsTriangle(Particle* p1, Particle* p2, Particle* p3);

	//calculates the normal of the triangle based on positions of the particles
	glm::vec3 calculateNormal();

	//calculates the velocity of the triangle based on the velocities of the particles
	glm::vec3 calculateVelocity();

	//calculates the area of the triangle
	float calculateArea();

	//method that applies aerodynamic force to the triangle and applies the force to each of the particles
	void applyAerodynamicForce(glm::vec3 wind);

	//update method that just gets a new normal
	void update();

	//adds normal to each of the particles it is connected to
	void addNormalToParticles();

	//returns a particle close to intersection point
	Particle* intersectedParticle(glm::vec3 rayDirection, glm::vec3 rayPoint);
};

