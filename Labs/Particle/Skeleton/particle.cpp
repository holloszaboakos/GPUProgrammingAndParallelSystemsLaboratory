#include "particle.hpp"
#include <stdio.h>
#include <random>
#include <ctime>  
#include <limits>

unsigned int Particle::size = 50;
float* Particle::position = new float[size * 4];
float* Particle::feedback = new float[size * 4];

Particle::Particle()
{
	srand(time(0));
	for (int i = 0; i < size; i++) {
		position[i * 4] = (rand() / (float)RAND_MAX) * 2 - 1;
		position[i * 4 + 1] = (rand() / (float)RAND_MAX) * 2 - 1;
		position[i * 4 + 2] = (rand() / (float)RAND_MAX) * 0.02 - 0.01;
		position[i * 4 + 3] = (rand() / (float)RAND_MAX) * 0.02 - 0.01;
	}
	printf("hadad%f\n", position[size * 4 - 1]);
}

Particle::~Particle()
{
	glDeleteBuffers(1, &bao1);
	glDeleteVertexArrays(1, &vao1);
	delete[]position;

}

void Particle::init()
{
	glGenVertexArrays(1, &vao1);
	glBindVertexArray(vao1);
	glGenBuffers(1, &bao1);
	glBindBuffer(GL_ARRAY_BUFFER, bao1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * size * 4, position, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer((GLuint)0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);
	glGenBuffers(1, &bao2);
	glBindBuffer(GL_ARRAY_BUFFER, bao2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * size * 4, nullptr, GL_STATIC_READ);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer((GLuint)0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	glGenQueries(1, &query);


}


void Particle::render() {
	glBindVertexArray(vao1);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, bao2);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, size);
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glFlush();
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(float) * size * 4, feedback); 
	GLuint primitives;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);
	printf("%u primitives written!\n\n", primitives);
	size = primitives;
	glBindVertexArray(0);
	glBindVertexArray(0);

	std::swap(vao2, vao1);
	std::swap(bao2, bao1);


}

