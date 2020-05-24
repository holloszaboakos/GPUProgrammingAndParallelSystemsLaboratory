// Skeleton.cpp : Defines the entry point for the console application.
//

#include <GL\glew.h>
#include <GL\freeglut.h>

#include <cstdio>

#include "shader.hpp"
#include "texture.hpp"
#include "quad.hpp"

const unsigned int windowWidth = 600;
const unsigned int windowHeight = 600;


struct vec2 {
	public: 
		float coords[2];
};
struct vec3 {
public:
	float coords[3];
};
struct catmulLine {
public:
	vec2 vecs[100];
	vec3 cols[100];
	int size = 0;
	GLuint array_id;
	GLuint vecbuff_id;
	GLuint colbuff_id;
	void add(vec2 v, vec3 c) {
		if (size == 100) return;
		vecs[size] = v;
		cols[size] = c;
		size++;
		load();
	}

	void init() {
		glGenVertexArrays(1, &array_id);
		glBindVertexArray(array_id);

		glGenBuffers(1, &vecbuff_id);
		glBindBuffer(GL_ARRAY_BUFFER, vecbuff_id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 0, NULL, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);

		/*glGenBuffers(1, &colbuff_id);
		glBindBuffer(GL_ARRAY_BUFFER, colbuff_id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 0, NULL, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);*/

		glBindVertexArray(0);
	}

	void load(){
		glBindVertexArray(array_id);
		
		glBindBuffer(GL_ARRAY_BUFFER, vecbuff_id);
		glBufferData(GL_ARRAY_BUFFER, size*2*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
		float* vertices = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		for (int i = 0; i < size; i++)
			for (int j = 0; j < 2; j++) 
				vertices[2 * i + j] = vecs[i].coords[j];
		glUnmapBuffer(GL_ARRAY_BUFFER);

		/*glBindBuffer(GL_ARRAY_BUFFER, colbuff_id);
		glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
		float* colors = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		for (int i = 0; i < size; i++)
			for (int j = 0; j < 3; j++)
				colors[3 * i + j] = cols[i].coords[j];
		glUnmapBuffer(GL_ARRAY_BUFFER);*/

		glBindVertexArray(0);
	}
	void render(){
		glBindVertexArray(array_id);
		glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, size);
		glBindVertexArray(0);
	}
};

Quad quad;
Shader shader;
Texture2D image;
catmulLine line;

void onInitialization()
{
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		printf("Cannot initialize GLEW\n");
		exit(-1);
	}

	glClearColor(0.1f, 0.0f, 0.0f, 1.0f);
	//quad.init();
	line.init();
	shader.loadShader(GL_VERTEX_SHADER, "..\\shaders\\passthrough.vert");
	shader.loadShader(GL_TESS_CONTROL_SHADER, "..\\shaders\\triang.tc");
	shader.loadShader(GL_TESS_EVALUATION_SHADER, "..\\shaders\\triang.te");
	//shader.loadShader(GL_GEOMETRY_SHADER, "..\\shaders\\catmul.geom");
	shader.loadShader(GL_FRAGMENT_SHADER, "..\\shaders\\simple.frag"); 

	shader.compile();

	//glProgramParameteri(shader.getProgramID(), GL_GEOMETRY_OUTPUT_TYPE, GL_LINE_STRIP);
	//glProgramParameteri(shader.getProgramID(), GL_GEOMETRY_VERTICES_OUT, 100);


	//image.initialize(100, 100);
	//image.loadFromFile("..\\..\\..\\Common\\images\\lena.jpg");
}

void onDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.enable();
	shader.setUniformTexture("data", image.getTextureHandle(), 0);
	//quad.render();
	line.render();
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

void onMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		line.add(
			vec2{ 
				-1+2*x / (float)windowWidth, 
				1-2*y / (float)windowHeight }, 
			vec3{ 1.0f, 1.0f, 1.0f }
		);
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

