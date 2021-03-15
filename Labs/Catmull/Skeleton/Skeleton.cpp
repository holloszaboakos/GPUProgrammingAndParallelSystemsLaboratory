// Skeleton.cpp : Defines the entry point for the console application.
//

#include <GL\glew.h>
#include <GL\freeglut.h>

#include <cstdio>

#include "shader.hpp"
#include "texture.hpp"
#include "catmull.hpp"

const unsigned int windowWidth = 600;
const unsigned int windowHeight = 600;

Catmull catmull;
Shader shader;
Texture2D image;

void onInitialization()
{
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		printf("Cannot initialize GLEW\n");
		exit(-1);
	}

	glClearColor(0.4f, 0.6f, 0.8f, 1.0f);
	catmull.init();
	glProgramParameteri((GLuint) &shader, GL_GEOMETRY_INPUT_TYPE, GL_POINTS);
	shader.loadShader(GL_VERTEX_SHADER, "..\\shaders\\catmull.vert");
	shader.loadShader(GL_GEOMETRY_SHADER, "..\\shaders\\catmull.geom");
	shader.loadShader(GL_FRAGMENT_SHADER, "..\\shaders\\catmull.frag");
	shader.compile();

	image.initialize(100, 100);
	image.loadFromFile("..\\..\\..\\Common\\images\\lena.jpg");
}

void onDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.enable();
	shader.setUniformTexture("data", image.getTextureHandle(), 0);
	catmull.render();
	shader.disable();

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

void onMouse(int button, int state, int mousex, int mousey) 
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			catmull.addPoint((mousex * 2.0f / windowWidth)-1.0f, (mousey * (-2.0f) / windowHeight) + 1.0f);
		}
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
	glutMainLoop();

    return 0;
}

