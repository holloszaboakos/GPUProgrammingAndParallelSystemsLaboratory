#version 430

in vec4 fVelocity;

out vec4 outColor;

void main()
{
	float xvel = (fVelocity.x + 1.0)/ 2.0;
	float yvel = (fVelocity.y + 1.0)/ 2.0;
	float zvel = (fVelocity.z + 1.0)/ 2.0;
	outColor = vec4(xvel,yvel,zvel, 1.0);
}