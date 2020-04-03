#include "Cloth.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

Cloth::Cloth(int width, int height) {
	this->width = width;
	this->height = height;
	force = glm::vec3(0);
	selectedParticle = NULL;
	initCloth();
}

void Cloth::initCloth() {
	//spring and particle properties
	particleSeperation = 0.1f;
	float springConstant = 5000.0f;
	float dampingConstant = 100.0f;
	float diagonalSeperation = glm::sqrt(particleSeperation * particleSeperation + particleSeperation * particleSeperation);

	//initialize particles
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			//create a new particle at a calculated position (the first row of particles will be set to fixed)
			float xPos = - (float)j * particleSeperation;
			float yPos = 6 - ((float)i * particleSeperation);
			float zPos = 0;
			glm::vec3 position = glm::vec3(xPos, yPos, zPos);
			Particle* particle = new Particle(0.15f, 0.f, 0.2f, 0.f, position, glm::vec3(0), i == 0 ? true : false);
			particles.push_back(particle);
		}
	}

	//initialize springs connected in squares
	for (int i = 0; i < height - 1; i++) {
		for (int j = 0; j < width - 1; j++) {
			Particle* p1;
			Particle* p2;
			SpringDamper* springDamper;

			//connect upper two springs (if first iteration)
			if (i == 0) {
				p1 = particles[width * i + j];
				p2 = particles[width * i + j + 1];
				springDamper = new SpringDamper(springConstant, dampingConstant, p1, p2);
				springDampers.push_back(springDamper);
			}

			//connect bottom two springs
			p1 = particles[width * (i+1) + j];
			p2 = particles[width * (i+1) + j + 1];
			springDamper = new SpringDamper(springConstant, dampingConstant, p1, p2);
			springDampers.push_back(springDamper);

			//connect left springs (if first row)
			if (j == 0) {
				p1 = particles[width * (i) + j];
				p2 = particles[width * (i + 1) + j];
				springDamper = new SpringDamper(springConstant, dampingConstant, p1, p2);
				springDampers.push_back(springDamper);
			}

			//connect right springs
			p1 = particles[width * (i) + j + 1];
			p2 = particles[width * (i + 1) + j + 1];
			springDamper = new SpringDamper(springConstant, dampingConstant, p1, p2);
			springDampers.push_back(springDamper);

			//connect diagonal one
			p1 = particles[width * (i) + j];
			p2 = particles[width * (i + 1) + j + 1];
			springDamper = new SpringDamper(springConstant, dampingConstant, p1, p2);
			springDampers.push_back(springDamper);

			//connect diagonal two
			p1 = particles[width * (i) + j + 1];
			p2 = particles[width * (i + 1) + j];
			springDamper = new SpringDamper(springConstant, dampingConstant, p1, p2);
			springDampers.push_back(springDamper);
		}
	}

	//initialize triangles
	for (int i = 0; i < height - 1; i++) {
		for (int j = 0; j < width - 1; j++) {
			//make first triangle
			Particle* p1 = particles[width * i + j];
			Particle* p2 = particles[width * (i+1) + j];
			Particle* p3 = particles[width * i + j + 1];
			PhysicsTriangle* pt = new PhysicsTriangle(p1, p2, p3);
			physicsTriangles.push_back(pt);

			//make second triangle
			p1 = particles[width * i + j + 1];
			p2 = particles[width * (i + 1) + j];
			p3 = particles[width * (i + 1) + j + 1];
			pt = new PhysicsTriangle(p1, p2, p3);
			physicsTriangles.push_back(pt);
		}
	}

	//initalize buffers
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &vbon);
	glGenBuffers(1, &ebo);

	//setup ebo for rendering triangle strips
	for (int i = 0; i < height - 1; i++) {
		for (int j = 0; j < width; j++) {
			int point = i * (width) + j;
			if (j == 0 && i > 0) {
				triangles.push_back(point);
			}

			point = i * (width) + j;
			triangles.push_back(point);
			point = (i + 1) * (width) + j;
			triangles.push_back(point);

			if (j == width - 1 && i < height - 2) {
				triangles.push_back(point);
			}
		}
	}

	//set size of vertex and normal vectors
	vertices.resize(particles.size());
	normals.resize(particles.size());

	//setup geometry data
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i] = particles[i]->getPosition();
	}
	initNormals();

	// Bind to the VAO.
	glBindVertexArray(vao);

	// Send vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	//Send normal data to gpu
	glBindBuffer(GL_ARRAY_BUFFER, vbon);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	//Send ebo data 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * triangles.size(), triangles.data(), GL_STATIC_DRAW);

	// Unbind from the VBO.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind from the VAO.
	glBindVertexArray(0);

	sendGraphicsData();
}

void Cloth::update(float deltaTime) {
	//update spring dampers
	for (int i = 0; i < springDampers.size(); i++) {
		springDampers[i]->update();
	}

	//update particles
	for (int i = 0; i < particles.size(); i++) {
		//add force to the particles first
		particles[i]->applyAcceleration(acceleration);

		//update particle positions 
		particles[i]->update(deltaTime);
	}

	////update physics triangles
	for (int i = 0; i < physicsTriangles.size(); i++) {
		physicsTriangles[i]->update();
	}

	//update geometry data
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i] = particles[i]->getPosition();
	}
	initNormals();

	//set force to 0
	force = glm::vec3(0);
	acceleration = glm::vec3(0);

	//send geometry data
	sendGraphicsData();
}

void Cloth::initNormals() {
	//FIRST METHOD USES SURROUNDING PARTICLES POSITIONS
	/*for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			glm::vec3 normal = calculateNormal(j, i);
			normals[width * i + j] = normal;
		}
	}*/

	//DIFFERENT METHOD USING TRIANGLE NORMALS
	//set normals to zero
	for (int i = 0; i < particles.size(); i++) {
		particles[i]->setNormal(glm::vec3(0));
	}

	//loop through triangles and set normal
	for (int i = 0; i < physicsTriangles.size(); i++) {
		physicsTriangles[i]->addNormalToParticles();
	}

	//normalize particle normals
	for (int i = 0; i < particles.size(); i++) {
		glm::vec3 pNormal = glm::normalize(particles[i]->getNormal());
		particles[i]->setNormal(pNormal);
	}

	//adjust normals vector accordingly
	for (int i = 0; i < particles.size(); i++) {
		normals[i] = particles[i]->getNormal();
	}
}

glm::vec3 Cloth::calculateNormal(int x, int z) {
	//Handle edge cases 
	if (x == 0) {
		x = 1;
	}
	if (z == 0) {
		z = 1;
	}
	if (x == width-1) {
		x--;
	}
	if (z == height-1) {
		z--;
	}

	//Get points of neighbours
	glm::vec3 hl = vertices.at((z) * (width) + (x - 1));
	glm::vec3 hr = vertices.at((z) * (width) + (x + 1));
	glm::vec3 hd = vertices.at((z + 1) * (width) + (x));
	glm::vec3 hu = vertices.at((z - 1) * (width) + (x));

	//Calculate normal based upon cross product
	glm::vec3 n = glm::normalize(glm::cross(hr - hl, hu - hd));

	return n;
}

void Cloth::sendGraphicsData() {
	// Bind to the VAO.
	glBindVertexArray(vao);

	//Send vertex data to gpu
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(),
		vertices.data(), GL_STATIC_DRAW);

	//Send normal data to gpu
	glBindBuffer(GL_ARRAY_BUFFER, vbon);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);

	// Unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Cloth::draw(glm::mat4 viewProjMtx, GLuint shader) {
	glm::mat4 modelMtx = glm::mat4(1);

	// Set up shader
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "ModelMtx"), 1, false, (float*)&modelMtx);

	glm::mat4 mvpMtx = viewProjMtx * modelMtx;
	glUniformMatrix4fv(glGetUniformLocation(shader, "ModelViewProjMtx"), 1, false, (float*)&mvpMtx);

	// Bind to the VAO.
	glBindVertexArray(vao);
	// Draw the model
	glDisable(GL_CULL_FACE);	//disable face culling so that both sides of cloth are rendered
	glDrawElements(GL_TRIANGLE_STRIP, triangles.size(), GL_UNSIGNED_INT, 0);
	glEnable(GL_CULL_FACE);
	// Unbind from the VAO.
	glBindVertexArray(0);

	glUseProgram(0);
}

void Cloth::printCloth(Cloth* cloth) {
	//print width and height
	std::cout << "WIDTH: " << cloth->width << std::endl;
	std::cout << "HEIGHT: " << cloth->height << std::endl;
	std::cout << "\n/////////////////////\n";

	//print triangle data for testing
	std::cout << "TRIANGLES: " << std::endl;
	for (int i = 0; i < cloth->triangles.size(); i++) {
		std::cout << cloth->triangles[i] << " ";
	}
	std::cout << "\n/////////////////////\n";

	//print vertex data
	std::cout << "VERTICES: " << std::endl;
	for (int i = 0; i < cloth->vertices.size(); i++) {
		std::cout << glm::to_string(cloth->vertices[i]) << std::endl;
	}
	std::cout << "\n/////////////////////\n";

	//print normal data
	std::cout << "NORMALS: " << std::endl;
	for (int i = 0; i < cloth->normals.size(); i++) {
		std::cout << glm::to_string(cloth->normals[i]) << std::endl;
	}
	std::cout << "\n/////////////////////\n";

	//print first particle data to check the forces
	std::cout << "PARTICLE ONE FORCE: " << std::endl;
	std::cout << glm::to_string(cloth->particles[0]->getForce()) << std::endl;
	std::cout << "\n/////////////////////\n";
}

void Cloth::applyForce(glm::vec3 force) {
	this->force += force;
}

void Cloth::applyAcceleration(glm::vec3 acceleration) {
	this->acceleration += acceleration;
}

void Cloth::movePosition(glm::vec3 deltaPosition) {
	//move top particles 
	for (int i = 0; i < width; i++) {
		particles[i]->movePosition(deltaPosition);
	}
}

void Cloth::applyAerodynamicForce(glm::vec3 wind) {
	//loop through physics triangles
	for (int i = 0; i < physicsTriangles.size(); i++) {
		physicsTriangles[i]->applyAerodynamicForce(wind);
	}
}

void Cloth::selectParticle(glm::vec3 rayDirection, glm::vec3 rayPoint) {
	//loop through physics triangles and test for intersection
	for (int i = 0; i < physicsTriangles.size(); i++) {
		Particle* pTemp;
		if ((pTemp = physicsTriangles[i]->intersectedParticle(rayDirection, rayPoint))) {
			selectedParticle = pTemp;
			selectedParticle->setFixed(true);
			
			//return when first selected particle is found
			//can be changed further down to get the closest particle to screen
			return;
		}
	}
}

void Cloth::deselectParticle() {
	if(selectedParticle != NULL)
		selectedParticle->setFixed(false);
	selectedParticle = NULL;
}

void Cloth::moveSelectedParticle(glm::vec3 deltaPosition) {
	if(selectedParticle != NULL)
		selectedParticle->movePosition(deltaPosition);
}

void Cloth::moveSelectedParticle(glm::vec3 ray, glm::vec3 rayPoint) {
	if (selectedParticle == NULL)
		return;
	//calculate delta position
	glm::vec3 rp = selectedParticle->getPosition() - rayPoint;
	glm::vec3 rn = glm::dot(rp, ray) * ray;
	selectedParticle->movePosition(rn - rp);
}

void Cloth::freeClothFixedPoints() {
	for (int i = 0; i < particles.size(); i++) {
		if (particles[i]->getFixed()) {
			particles[i]->setFixed(false);
		}
	}
}

void Cloth::Reset() {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			//resets the position of particles
			float xPos = -(float)j * particleSeperation;
			float yPos = 6 - ((float)i * particleSeperation);
			float zPos = 0;
			glm::vec3 position = glm::vec3(xPos, yPos, zPos);
			int index = i * width + j;
			particles[index]->setPosition(position);
			particles[index]->setVelocity(glm::vec3(0));

			//sets first row to be fixed
			if (i == 0) {
				particles[index]->setFixed(true);
			}
		}
	}
}