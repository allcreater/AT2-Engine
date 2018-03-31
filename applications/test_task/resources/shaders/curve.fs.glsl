#version 420 core

precision mediump float;

uniform vec4 u_Color;

//in vec4 v_Color;

layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = u_Color;
}