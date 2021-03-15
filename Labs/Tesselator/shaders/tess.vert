#version 330

in vec3 vPosition;
out vec3 tPosition;
uniform sampler2D data;

void main()
{
	tPosition = vec3(
		vPosition.xy, 
		dot(
			texture(data, -1 + vPosition.xy/ 4 - vec2(0.5,0.5)),
			vec4(0.21,0.39,0.4,0.0)
			) / 4
	 ) ;
}