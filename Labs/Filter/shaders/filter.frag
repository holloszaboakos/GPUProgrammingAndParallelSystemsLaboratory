#version 330

uniform sampler2D data;
uniform int textureHeight;
uniform int textureWidth;
uniform int mode;

in vec2 fTexCoord;
out vec4 outColor;

void main()
{
	float verticalStep=1.0/textureHeight;
	float horizontalStep=1.0/textureHeight;
	if(mode==0){
		//Black and with
		float intensity = dot(texture(data, fTexCoord),vec4(0.21,0.39,0.4,0.0));
		outColor = vec4(intensity,intensity,intensity,1.0);

	}
	else if(mode==1){
		//thresholding
		float intensity = dot(texture(data, fTexCoord),vec4(0.21,0.39,0.4,0.0));
		if(intensity<0.5)
			outColor = vec4(0,0,0,1.0);
		else
			outColor = vec4(1.0,1.0,1.0,1.0);
		
	}
	else if(mode==2){
		//previtt
		vec3 upper = vec3(
			dot(texture(data, fTexCoord + vec2(-horizontalStep,verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(0.0,verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(horizontalStep,verticalStep)),vec4(0.21,0.39,0.4,0.0))
		);
		vec3 middle = vec3(
			dot(texture(data, fTexCoord + vec2(-horizontalStep,0.0)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(0.0,0.0)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(horizontalStep,0.0)),vec4(0.21,0.39,0.4,0.0))
		);
		vec3 lower = vec3(
			dot(texture(data, fTexCoord + vec2(-horizontalStep,-verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(0.0,-verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(horizontalStep,-verticalStep)),vec4(0.21,0.39,0.4,0.0))
		);
		float intensityX = upper.x + middle.x + lower.x - (upper.z + middle.z + lower.z);
		float intensityY = upper.x + upper.y + upper.z - (lower.x + lower.y + lower.z);
		float intensity = length(vec2(intensityX,intensityY));
		outColor = vec4(intensity,intensity,intensity,1.0);
	}
	else if(mode==3){
		//sobel
		vec3 upper = vec3(
			dot(texture(data, fTexCoord + vec2(-horizontalStep,verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(0.0,verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(horizontalStep,verticalStep)),vec4(0.21,0.39,0.4,0.0))
		);
		vec3 middle = vec3(
			dot(texture(data, fTexCoord + vec2(-horizontalStep,0.0)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(0.0,0.0)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(horizontalStep,0.0)),vec4(0.21,0.39,0.4,0.0))
		);
		vec3 lower = vec3(
			dot(texture(data, fTexCoord + vec2(-horizontalStep,-verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(0.0,-verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(horizontalStep,-verticalStep)),vec4(0.21,0.39,0.4,0.0))
		);
		float intensityX = upper.x + 2*middle.x + lower.x - (upper.z + 2*middle.z + lower.z);
		float intensityY = upper.x + 2*upper.y + upper.z - (lower.x + 2*lower.y + lower.z);
		float intensity = length(vec2(intensityX,intensityY));
		outColor = vec4(intensity,intensity,intensity,1.0);
	}
	else if(mode==4){
		//laplace
		vec3 upper = vec3(
			dot(texture(data, fTexCoord + vec2(-horizontalStep,verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(0.0,verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(horizontalStep,verticalStep)),vec4(0.21,0.39,0.4,0.0))
		);
		vec3 middle = vec3(
			dot(texture(data, fTexCoord + vec2(-horizontalStep,0.0)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(0.0,0.0)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(horizontalStep,0.0)),vec4(0.21,0.39,0.4,0.0))
		);
		vec3 lower = vec3(
			dot(texture(data, fTexCoord + vec2(-horizontalStep,-verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(0.0,-verticalStep)),vec4(0.21,0.39,0.4,0.0)),
			dot(texture(data, fTexCoord + vec2(horizontalStep,-verticalStep)),vec4(0.21,0.39,0.4,0.0))
		);
		float intensity = upper.y + middle.x + middle.z + lower.y - 4.0 * middle.y;
		outColor = vec4(intensity,intensity,intensity,1.0);
	}
}
