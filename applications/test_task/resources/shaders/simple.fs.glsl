#version 420 core

precision mediump float;

in vec2 v_texCoord;

layout (location = 0) out vec4 FragColor;

void main()
{
	//vec4 color = texture(u_colorMap, v_texCoord);
	FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}