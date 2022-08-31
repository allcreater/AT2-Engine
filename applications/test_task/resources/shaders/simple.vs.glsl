#version 410 core

uniform mat4 u_matProjection;

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec4 a_Color;

out vec4 v_Color;

void main()
{
	gl_Position = u_matProjection * vec4(a_Position, 0.0, 1.0);
	v_Color = a_Color;
}
