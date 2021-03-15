#version 330

uniform sampler2D data;

in vec2 fTexCoord;
out vec4 outColor;

void main()
{
	vec2 c = fTexCoord * 4 - vec2(2.0,2.0);
	//eltolás és skálázás a szép képért
	vec2 xn = vec2(0.0,0.0);
	float limit = 1000000.0;
	for(int iter = 0;iter<100000;iter++){
		//xnp kiszámítása xn-ből
		xn = vec2(xn.x*xn.x-xn.y*xn.y,2*xn.x*xn.y);
		xn = xn + c;
		//határ vizsgálat
		if(length(xn)>=limit)
			break;
	} 
	if(length(xn)<=limit)
		outColor = vec4(abs(sin(length(xn)*xn.x)),abs(cos(length(xn)*xn.y)),abs(sin(length(xn))),1.000);
	else
		outColor = vec4(limit/length(xn),limit/length(xn),limit/length(xn),1.0);
}
