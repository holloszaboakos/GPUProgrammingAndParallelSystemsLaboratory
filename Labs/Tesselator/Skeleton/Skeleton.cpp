// Skeleton.cpp : Defines the entry point for the console application.
//

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>
#include <cmath> 
#include <algorithm>

#include "shader.hpp"
#include "texture.hpp"
#include "DebugOpenGL.hpp"
#include "camera.h"

using namespace std;
const unsigned int windowWidth = 600;
const unsigned int windowHeight = 600;

Shader shader;

GLuint vao;
GLuint vertexBuffer;
Texture2D image;
Camera camera;

int tessLevelInner = 1;
int tessLevelOuter = 1;

const float vertices[12] = {
	-2.0f, -2.0f, 0.0f,
	-2.0f, 2.0f, 0.0f,
	2.0f, 2.0f, 0.0f,
	2.0f, -2.0f, 0.0f
};

glm::mat4 perspective = glm::perspective(
	glm::radians(90.0f), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
	4.0f / 3.0f,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
	0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
	100.0f             // Far clipping plane. Keep as little as possible.
);;

void onInitialization()
{
	tessLevelInner = (int)(100 / glm::length(glm::vec3(0,0,-1)-camera.Position));
	
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

	shader.loadShader(GL_VERTEX_SHADER, "../shaders/tess.vert");
	shader.loadShader(GL_TESS_CONTROL_SHADER, "../shaders/tess.tc");
	shader.loadShader(GL_TESS_EVALUATION_SHADER, "../shaders/tess.te");
	shader.loadShader(GL_FRAGMENT_SHADER, "../shaders/tess.frag");
	shader.compile();

	image.initialize(100, 100);
	image.loadFromFile("..\\..\\..\\Common\\images\\lena.jpg");

	// Single triangle patch Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

void onDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 MV = perspective * camera.GetViewMatrix();

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	shader.enable();
	shader.setUniformTexture("data", image.getTextureHandle(), 0);
	shader.setUniformMat4("MV", MV);
	shader.setUniform1i("tessLevelInner", tessLevelInner);
	shader.setUniform1i("tessLevelOuter", tessLevelOuter);
	glBindVertexArray(vao);
	glDrawArrays(GL_PATCHES, 0, 4);
	glBindVertexArray(0);
	shader.disable();
	
	glutSwapBuffers();
	glutPostRedisplay();
}

void onKeyboard(unsigned char key, int pX, int pY) {
	glm::vec3 moveDirection;
	switch (key)
	{
	case 27:
		glutExit();
		break;

	case 'w':
		camera.ProcessKeyboard(Camera_Movement::FORWARD, 0.1);
		break;

	case 's':
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, 0.1);
		break;

	case 'd':
		camera.ProcessKeyboard(Camera_Movement::RIGHT, 0.1);
		break;

	case 'a':
		camera.ProcessKeyboard(Camera_Movement::LEFT, 0.1);
		break;
	}
	tessLevelInner = (int)(100 / glm::length(glm::vec3(0, 0, -1) - camera.Position));
}

void onMouse(int button, int state, int x, int y) {
	camera.ProcessMouseMovement((x/(float)windowWidth*2-1)*100,(y / (float)windowHeight * 2 - 1) * 100);
	tessLevelInner = (int)(100 / glm::length(glm::vec3(0, 0, -1) - camera.Position));
	glutPostRedisplay();
}

void onMouseWheel(int button, int dir, int x, int y) {
	camera.ProcessMouseScroll(dir);
	glutPostRedisplay();
	tessLevelInner = (int)(100 / glm::length(glm::vec3(0, 0, -1) - camera.Position));
}

void onIdle()
{
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(3, 3);
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
	glutMouseFunc(onMouse);
	glutMouseWheelFunc(onMouseWheel);
	glutIdleFunc(onIdle);
	glutMainLoop();

    return 0;
}

