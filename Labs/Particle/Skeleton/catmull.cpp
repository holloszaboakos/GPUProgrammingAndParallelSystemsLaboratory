#include "catmull.hpp"

unsigned int  Catmull::count = 0;
GLfloat Catmull::controlPoints[200] = {};

GLfloat Catmull::colors[300] = {};

Catmull::Catmull()
{
}

Catmull::~Catmull()
{
	glDeleteBuffers(1, &controlPointBuffer);
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteVertexArrays(1, &colorBuffer);
}


void Catmull::addPoint(float x, float y) {
	if (count ==0) {
		controlPoints[0] = x;
		controlPoints[1] = y;
		controlPoints[2] = x;
		controlPoints[3] = y;
		count += 1;
		init();
	}
	else if (count <= 98) {
		controlPoints[2 * count + 2] = x;
		controlPoints[2 * count + 3] = y;
		controlPoints[2 * count + 4] = x;
		controlPoints[2 * count + 5] = y;
		count += 1;
		init();
	}
}

void Catmull::init()
{
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glGenBuffers(1, &controlPointBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, controlPointBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * (count+2) * 2, controlPoints, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * (count + 2) * 3, colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

void Catmull::render() {
	glBindVertexArray(vertexArray);
	glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, count + 2);
	glBindVertexArray(0);
}

