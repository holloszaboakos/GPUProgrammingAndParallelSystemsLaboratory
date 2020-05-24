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
#include <time.h>

const unsigned int windowWidth = 600;
const unsigned int windowHeight = 600;

Shader shader;

GLuint vao;
GLuint vertexBuffer;

int tessLevelInner = 1;
int tessLevelOuter = 1;

const unsigned int size = 20;

float vertices[size * size *4*3];

const float quad_vertices[4 * 4] = {
	0.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, -1.0f, 1.0f,
	0.75f, 0.75f, 0.0f, 1.0f,
	1.0f, 0.0f, -1.0f, 1.0f,
};

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

	glClearColor(0.1f, 0.1f, 0.5f, 1.0f);

	shader.loadShader(GL_VERTEX_SHADER, "../shaders/tess.vert");
	shader.loadShader(GL_TESS_CONTROL_SHADER, "../shaders/tess.tc");
	shader.loadShader(GL_TESS_EVALUATION_SHADER, "../shaders/tess.te");
	shader.loadShader(GL_FRAGMENT_SHADER, "../shaders/tess.frag");
	shader.compile();

	time_t t;
	srand((unsigned)time(&t));

	for(int line =0 ; line < size; line++)
		for (int col = 0; col < size; col++)
			for (int cor = 0; cor < 4; cor++) {
				int idx = (line * size + col) * 4 + cor;
					vertices[idx * 3 + 0] = 1.0f / size * (col + cor / 2);
					vertices[idx * 3 + 1] = 1.0f/ size * (line + (cor + 1) % 4 / 2);

					if (col != 0 && cor==0) {
						vertices[idx * 3 + 2] = vertices[(idx-1) * 3 + 2];
					}
					else if (col != 0 && cor == 1) {
						vertices[idx * 3 + 2] = vertices[(idx - 3) * 3 + 2];
					}
					else if (line != 0 && cor == 0) {
						vertices[idx * 3 + 2] = vertices[(idx - size * 4 + 1) * 3 + 2];
					}
					else if (line != 0 && cor == 3) {
						vertices[idx * 3 + 2] = vertices[(idx - size * 4 - 1) * 3 + 2];
					}
					else
						vertices[idx * 3 + 2] = rand() % size / -200.0f;
			}

	// Single triangle patch Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 100* 100* 4 * 3, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

void onDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 MV = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Ezért csak körvonal

	shader.enable();
	shader.setUniformMat4("MV", MV);
	shader.setUniform1i("tessLevelInner", tessLevelInner);
	shader.setUniform1i("tessLevelOuter", tessLevelOuter);
	glBindVertexArray(vao);
	glDrawArrays(GL_PATCHES, 0, 100 * 100 * 4);
	glBindVertexArray(0);
	shader.disable();
	
	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
	switch (key)
	{
	case 27:
		glutExit();
		break;

	case '[':
		tessLevelInner = std::max(1, tessLevelInner - 1);
		break;

	case ']':
		tessLevelInner = std::min(64, tessLevelInner + 1);
		break;

	case '{':
		tessLevelOuter = std::max(1, tessLevelOuter - 1);
		break;

	case '}':
		tessLevelOuter = std::min(64, tessLevelOuter + 1);
		break;
	}
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
	glutIdleFunc(onIdle);
	glutMainLoop();

    return 0;
}

