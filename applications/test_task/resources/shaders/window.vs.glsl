#version 420 core

layout(location = 0) in vec3 a_Position;

out vec2 v_texCoord;

void main()
{
	vec3 pos = a_Position*0.95f;
	gl_Position = vec4(pos, 1.0);
	
	v_texCoord = pos.xy/2.0 + vec2(0.5);
}
