#version 420 core

precision mediump float;

in vec2 v_texCoord;

layout (location = 0) out vec4 FragColor;

uniform vec4 u_Color;

void main()
{
	//vec4 color = texture(u_colorMap, v_texCoord);

	FragColor = u_Color * vec4(vec3(0.3), 0.7);
}