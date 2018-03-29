#version 420 core

layout(location = 1) in vec3 a_Position;

out vec2 v_texCoord;

void main()
{
	gl_Position = vec4(a_Position, 1.0);
	
	v_texCoord = a_Position.xy/2.0 + vec2(0.5);
}
