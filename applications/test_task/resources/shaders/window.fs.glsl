#version 420 core

precision mediump float;

in vec2 v_texCoord;

layout (location = 0) out vec4 FragColor;

uniform vec4 u_Color;
uniform sampler2D u_backgroundTexture;

void main()
{
	FragColor = u_Color * vec4(vec3(0.3), 0.7);
}