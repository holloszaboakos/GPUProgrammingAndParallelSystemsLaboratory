
#include <GL/glew.h>

class Catmull {
private:
    GLuint vertexArray;

    static GLfloat controlPoints[200];
    GLuint controlPointBuffer;
    static unsigned int count;

    static GLfloat colors[300];
    GLuint colorBuffer;

public:
    Catmull();
    ~Catmull();
    void addPoint(float x, float y);

    void init();
    void render();
};