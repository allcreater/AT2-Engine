#version 400 core

layout(location = 1) in vec4 a_Position;

uniform mat4 u_matMW, u_matProj;
uniform float u_phase;

out vec4 v_color;
out vec2 v_texCoord;

void main()
{
	gl_Position = a_Position;
	
	v_color = vec4(0.5);
	v_texCoord = a_Position.xy/2.0 + vec2(0.5);
}
