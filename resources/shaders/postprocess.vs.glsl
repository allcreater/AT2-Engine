#version 400 core

layout(location = 1) in vec4 a_Position;

out vec2 v_texCoord;

void main()
{
	gl_Position = a_Position;
	
	v_texCoord = a_Position.xy/2.0 + vec2(0.5);
}
