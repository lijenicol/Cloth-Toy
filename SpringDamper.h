#pragma once
#include <glm/glm.hpp>
#include "Particle.h"

class SpringDamper
{
private:
	Particle* connectedParticles[2];
	float springConstant, dampingConstant;
	float restLength;
	void calculateParticleForce();
public:
	SpringDamper(float springConstant, float dampingConstant, float restLength, Particle* particleOne, Particle* particleTwo);
	SpringDamper(float springConstant, float dampingConstant, Particle* particleOne, Particle* particleTwo);
	void update();
	void destroy();
};

