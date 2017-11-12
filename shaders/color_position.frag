#version 430


out vec4 outColor;

in vec3 oldPos;
in vec3 newNormal;
in vec2 texCoord;
uniform float time;


void main()
{


	vec3 newPos = oldPos + vec3(0.5);
	outColor = vec4(newPos, 1.0);

} 