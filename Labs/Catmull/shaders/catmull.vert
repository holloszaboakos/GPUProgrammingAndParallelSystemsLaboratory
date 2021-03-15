#version 330

in vec2 vControlPoint;
in vec3 vColor;

out vec3 gColor;

void main(void) {
   gl_Position = vec4(vControlPoint.x,vControlPoint.y,0.0,0.0);
   gColor = vColor;
}

