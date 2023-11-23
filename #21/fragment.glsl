#version 330 core

out vec4 fColor;
in vec3 outColor;

void main()
{
	fColor = vec4(outColor, 1.0);
}