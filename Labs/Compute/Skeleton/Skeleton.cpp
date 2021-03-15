// Skeleton.cpp : Defines the entry point for the console application.
//

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>
#include <algorithm>

#include "shader.hpp"
#include "texture.hpp"
#include "DebugOpenGL.hpp"

const unsigned int windowWidth = 600;
const unsigned int windowHeight = 600;

// Worker threads per workgroup
const unsigned int workGroupSize = 256;
// Number of particles
const unsigned int particlesNum = 1024;

// Vec4 like structure
struct xyzw
{
	float x, y, z, w;
};

// Particle movement shader (compute shader)
Shader particleMoveShader;
// Particle renderer shaders (standard pipeline)
Shader particleRenderShader;

// Position buffer
GLuint positionBuffer;
// Velocity buffer
GLuint velocityBuffer;

// Vertex array object
GLuint vao;

glm::vec2 gravityPoint = glm::vec2(0.0f, 0.0f);

void onInitialization()
{
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		printf("Cannot initialize GLEW\n");
		exit(-1);
	}
	glGetError();

	DebugOpenGL::init();
	DebugOpenGL::enableLowSeverityMessages(false);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	particleMoveShader.loadShader(GL_COMPUTE_SHADER, "../shaders/particle.comp");
	particleMoveShader.compile();

	particleRenderShader.loadShader(GL_VERTEX_SHADER, "../shaders/particle.vert");
	particleRenderShader.loadShader(GL_FRAGMENT_SHADER, "../shaders/particle.frag");
	particleRenderShader.compile();

	// Initialize the particle position buffer
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particlesNum * sizeof(xyzw), NULL, GL_STATIC_DRAW);
	xyzw* pos = (xyzw*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particlesNum * sizeof(xyzw), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (unsigned int i = 0; i < particlesNum; ++i)
	{
		pos[i].x = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		pos[i].y = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		pos[i].z = 0.0;// 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		pos[i].w = 1.0f;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// Initialize the particle velocity buffer
	glGenBuffers(1, &velocityBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocityBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particlesNum * sizeof(xyzw), NULL, GL_STATIC_DRAW);
	xyzw* vel = (xyzw*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particlesNum * sizeof(xyzw), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (unsigned int i = 0; i < particlesNum; ++i)
	{
		vel[i].x = 0.02f * ((float)rand() / (float)RAND_MAX) - 0.01f;
		vel[i].y = 0.02f * ((float)rand() / (float)RAND_MAX) - 0.01f;
		vel[i].z = 0.0;//2f * ((float)rand() / (float)RAND_MAX) - 0.01f;
		vel[i].w = 1.0f;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Initialize the vertex array object with the position and velocity buffers
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glVertexAttribPointer((GLuint)0, 4, GL_FLOAT, GL_FALSE, sizeof(xyzw), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer);
	glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, GL_FALSE, sizeof(xyzw), (GLvoid*)0);

	glBindVertexArray(0);

	// Set point primitive size
	glPointSize(4.0f);
}

void onDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update position and velocity
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocityBuffer);

	particleMoveShader.enable();
	particleMoveShader.setUniform2f("gravityPoint",gravityPoint.x, gravityPoint.y);
	glDispatchCompute(particlesNum / workGroupSize, 1, 1);

	// Synchronize between the compute and render shaders
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

	// Render the particles
	particleRenderShader.enable();
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, particlesNum);
	glBindVertexArray(0);
	particleRenderShader.disable();

	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
	switch (key)
	{
	case 27:
		glutExit();
		break;
	}
}

void onMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		gravityPoint = glm::vec2((x / (float)windowWidth) * 2 - 1, -((y / (float)windowHeight) * 2 - 1));
}

void onIdle()
{
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(argv[0]);
	glewExperimental = true;
	glewInit();

	printf("GL Vendor    : %s\n", glGetString(GL_VENDOR));
	printf("GL Renderer  : %s\n", glGetString(GL_RENDERER));
	printf("GL Version (string)  : %s\n", glGetString(GL_VERSION));
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("GL Version (integer) : %d.%d\n", major, minor);
	printf("GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	onInitialization();
	glutDisplayFunc(onDisplay);
	glutMouseFunc(onMouse);
	glutKeyboardFunc(onKeyboard);
	glutIdleFunc(onIdle);
	glutMainLoop();

	return 0;
}

