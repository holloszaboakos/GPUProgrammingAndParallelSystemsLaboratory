#version 430

in vec3 tePosition;
out vec4 outColor;

void main()
{
	vec3 dist = tePosition - vec3(0.5f,0.5f,1);
	float height = sqrt(dot(dist,dist)) / 2;
	outColor = vec4(height, height*height, height*height*height, 1.0);
}