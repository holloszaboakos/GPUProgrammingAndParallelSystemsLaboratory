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

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cuda_gl_interop.h>

const unsigned int windowWidth = 600;
const unsigned int windowHeight = 600;

// Number of particles
const unsigned int particlesNum = 256;

float posX;
float posY;

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

cudaGraphicsResource_t p_res = 0;
cudaGraphicsResource_t v_res = 0;

__device__ float lengthVec(float* vector) {
	float sum;
	for (int index = 0; index < 3; index++)
		sum += vector[index] * vector[index];
	return sqrt(sum);
}

__device__ float* minusVec(float* output, float* leftVector, float* rightVector) {
	for (int index = 0; index < 3; index++)
		output[index] = leftVector[index] - rightVector[index];
	return output;
}

__device__ float* plusVec(float* output, float* leftVector, float* rightVector) {
	for (int index = 0; index < 3; index++)
		output[index] = leftVector[index] + rightVector[index];
	return output;
}

__device__ float* timesVec(float* output, float* leftVector, float rightValue) {
	for (int index = 0; index < 3; index++)
		output[index] = leftVector[index] * rightValue;
	return output;
}

__device__ float* assignVec(float* leftVector, float* rightVector) {
	for (int index = 0; index < 3; index++)
		leftVector[index] = rightVector[index];
	return leftVector;
}

__global__ void moveKernel(float* positionArray, float* velocityArray, int size) {

	int threadIndex = threadIdx.x + blockIdx.x * blockDim.x;
	int startIndex = threadIndex * 4;

	const float dt = 0.0004;

	float newVelocity[3];
	assignVec(newVelocity, velocityArray + startIndex);
	float newPosition[3];
	float movement[3];
	plusVec(newPosition,
		positionArray + startIndex,
		timesVec(movement,
			newVelocity,
			dt
		)
	);


	bool shouldStepBack = false;
	for (int coordIndex = 0; coordIndex < 3; coordIndex++) {
		if (newPosition[coordIndex] <= -2.0 || newPosition[coordIndex] >= 2.0) {
			shouldStepBack = true;
			newVelocity[coordIndex] *= -1;
		}
	}

	if (shouldStepBack) {
		timesVec(newVelocity, newVelocity, 0.1);
		minusVec(newPosition, newPosition, movement);
	}


	for (int positionIndex = 0; positionIndex < size * 4; positionIndex += 4) {
		float acceleration[3];
		float epsilon = 0.01;

		minusVec(acceleration,
			positionArray + positionIndex,
			newPosition
		);

		float distance = lengthVec(acceleration);

		float temp = (distance * distance) + epsilon * epsilon;

		plusVec(newVelocity,
			newVelocity,
			timesVec(acceleration,
				acceleration,
				0.5 * dt / sqrt(temp * temp * temp)
			)
		);
	}

	__syncthreads();
	assignVec(positionArray + startIndex, newPosition);
	assignVec(velocityArray + startIndex, newVelocity);
}





void onInitialization()
{
	cudaGLSetGLDevice(0);
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

	//particleMoveShader.loadShader(GL_COMPUTE_SHADER, "../shaders/particle.comp");
	//particleMoveShader.compile();

	particleRenderShader.loadShader(GL_VERTEX_SHADER, "../shaders/particle.vert");
	particleRenderShader.loadShader(GL_FRAGMENT_SHADER, "../shaders/particle.frag");
	particleRenderShader.compile();

	// Initialize the particle position buffer
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particlesNum * sizeof(xyzw), NULL, GL_DYNAMIC_DRAW);
	xyzw* pos = (xyzw*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particlesNum * sizeof(xyzw), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (unsigned int i = 0; i < particlesNum; ++i)
	{
		pos[i].x = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		pos[i].y = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		pos[i].z = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		pos[i].w = 1.0f;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// Initialize the particle velocity buffer
	glGenBuffers(1, &velocityBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocityBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particlesNum * sizeof(xyzw), NULL, GL_DYNAMIC_DRAW);
	xyzw* vel = (xyzw*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, particlesNum * sizeof(xyzw), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (unsigned int i = 0; i < particlesNum; ++i)
	{
		vel[i].x = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		vel[i].y = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		vel[i].z = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		vel[i].w = 0.0;
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

	void* dev_p = 0;
	void* dev_v = 0;

	size_t psize;
	size_t vsize;

	cudaGraphicsGLRegisterBuffer(&p_res, positionBuffer, cudaGraphicsRegisterFlagsNone);
	cudaGraphicsGLRegisterBuffer(&v_res, velocityBuffer, cudaGraphicsRegisterFlagsNone);

	cudaGraphicsMapResources(1, &p_res);
	cudaGraphicsMapResources(1, &v_res);

	cudaGraphicsResourceGetMappedPointer(&dev_p, &psize, p_res);
	cudaGraphicsResourceGetMappedPointer(&dev_v, &vsize, v_res);

	moveKernel << < particlesNum / 256, 256 >> > ((float*)dev_p, (float*)dev_v, particlesNum);

	float* lol = new float[particlesNum * 4];
	cudaMemcpy(lol, dev_v, sizeof(float) * particlesNum * 4, cudaMemcpyDeviceToHost);

	//for (int i = 0; i < particlesNum * 4; i++)
	//	printf("%f\n", lol[i]);

	delete[] lol;


	cudaGraphicsUnmapResources(1, &p_res);
	cudaGraphicsUnmapResources(1, &v_res);

	cudaDeviceSynchronize();
	glFlush();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Update position and velocity

	//Create CUDA variables
	glBindVertexArray(vao);
	// Render the particles
	particleRenderShader.enable();
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

void onDrag(int x, int y) {
	posX = (float(x) / glutGet(GLUT_WINDOW_WIDTH)) * 2.0f - 1.0f;
	posY = -((float(y) / glutGet(GLUT_WINDOW_HEIGHT)) * 2.0f - 1.0f);
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
	glutKeyboardFunc(onKeyboard);
	//glutMotionFunc(onDrag);
	//glutPassiveMotionFunc(onDrag);
	glutIdleFunc(onIdle);
	glutMainLoop();

	return 0;
}

