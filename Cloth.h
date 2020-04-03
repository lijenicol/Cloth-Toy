#pragma once
#include "GL/glew.h"
#include <glm/glm.hpp>
#include <vector>
#include "Particle.h"
#include "SpringDamper.h"
#include "PhysicsTriangle.h"

class Cloth
{
private:
	//physics properties
	std::vector<Particle*> particles;
	std::vector<SpringDamper*> springDampers;
	std::vector<PhysicsTriangle*> physicsTriangles;
	float particleSeperation;

	//geometry properties
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> triangles;
	GLuint vao, vbo, vbon, ebo;

	//cloth properties
	int width, height;

	//initializes cloth geometry and physics data
	void initCloth();

	//sends data to gpu for rendering
	void sendGraphicsData();
	
	//calculates normals every frame (this may be really slow)
	void initNormals();
	glm::vec3 calculateNormal(int x, int z);

	//add force & acceleration property as it is a physics object
	glm::vec3 force, acceleration;

	//interactive properties
	Particle* selectedParticle;
public:
	Cloth(int width, int height);

	//updates physics and geometry properties of the cloth
	void update(float deltaTime);

	void draw(glm::mat4 viewProjMtx, GLuint shader);
	
	//used for debugging cloth geometry
	static void printCloth(Cloth* cloth);

	//adds force to the cloth so that it moves
	void applyForce(glm::vec3 force);

	//apply aerodynamic force to cloth
	void applyAerodynamicForce(glm::vec3 wind);

	//apply an acceleration to the cloth (for uniform gravity)
	void applyAcceleration(glm::vec3 acceleration);

	//moves cloth by position
	void movePosition(glm::vec3 deltaPosition);

	//checks if ray intersects cloth and selects corresponding particle
	void selectParticle(glm::vec3 rayDirection, glm::vec3 rayPoint);

	//deselects the currently selected particle
	void deselectParticle();

	//moves the selected particle by some amount
	void moveSelectedParticle(glm::vec3 deltaPosition);

	//moves selected particle based on rays
	void moveSelectedParticle(glm::vec3 ray, glm::vec3 rayPoint);

	//frees the cloth from hanging
	void freeClothFixedPoints();

	//resets particles of cloth
	void Reset();
};

