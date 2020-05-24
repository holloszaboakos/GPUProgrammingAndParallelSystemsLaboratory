#version 330

in vec2 vec;
//in vec3 col;

void main(void) {
   gl_Position = vec4(vec.x,vec.y,0,1);
}