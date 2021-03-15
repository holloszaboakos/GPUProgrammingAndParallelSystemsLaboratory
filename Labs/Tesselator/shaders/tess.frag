#version 430

out vec4 outColor;

uniform sampler2D data;
in vec3 tePosition;

void main()
{
	float intensity = dot(texture(data, tePosition.xy/ 4 - vec2(0.5,0.5)),vec4(0.21,0.39,0.4,0.0));
	outColor = vec4(intensity,intensity,intensity,1.0);
}