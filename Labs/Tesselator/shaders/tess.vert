#version 330

in vec4 vPosition;
out vec4 tPosition;

void main()
{
	tPosition = vPosition * mat4( 1,0,0,0, 0, cos(1.0f), -sin(1.0f), 0, 0, sin( 1.0f ), cos(1.0f), 0, 0,0,0,1  );
}