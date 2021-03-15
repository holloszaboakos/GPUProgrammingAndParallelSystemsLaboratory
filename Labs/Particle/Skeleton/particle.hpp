
#include <GL/glew.h>

class Particle {
private:
    static unsigned int size;

    GLuint bao1;
    GLuint vao1;
    GLuint bao2;
    GLuint vao2; 
    GLuint query;

    static float* position;
    static float* feedback;

public:
    Particle();
    ~Particle();

    void init();
    void render();
};