#version 420 core

layout(location = 0) in vec4 a_Color;
layout(location = 1) in vec2 a_Position;

out vec4 v_Color;

void main()
{
	gl_Position = vec4(a_Position, 0.0, 1.0);
	v_Color = a_Color;
}
