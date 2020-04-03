#pragma once
#include <glm/glm.hpp>
class Particle
{
private:
	//particle properties
	float lifeTime, currentLife;
	float mass, size, elasticity;
	glm::vec3 position, velocity, acceleration;
	glm::vec3 force;
	
	//store a normal with the particle which can be used in triangle rendering
	glm::vec3 normal;
	
	//interactive properties
	bool fixed;

	//method for calculating new position of a particle
	void calculatePosition(float deltaTime);

	//calculates the acceleration of the particle using Newtonian mechanics
	glm::vec3 calculateAcceleration();

	//calculates the velocity of the particle
	glm::vec3 calculateVelocity(float deltaTime);
public:
	void update(float deltaTime);
	void destroy();

	//applies force to particle
	void applyForce(glm::vec3 force);

	//applies acceleration to particle
	void applyAcceleration(glm::vec3 acceleration);

	//moves position of particle
	void movePosition(glm::vec3 deltaPosition);

	//sets position of particle
	void setPosition(glm::vec3 position);

	//sets velocity of particle
	void setVelocity(glm::vec3 velocity);

	bool getFixed();
	void setFixed(bool fixed);

	glm::vec3 getForce();
	glm::vec3 getPosition();
	glm::vec3 getVelocity();

	void addNormal(glm::vec3 normal);
	void setNormal(glm::vec3 normal);
	glm::vec3 getNormal();

	Particle(float mass, float size, float elasticity, float lifeTime, glm::vec3 position, glm::vec3 velocity, bool fixed);
};

