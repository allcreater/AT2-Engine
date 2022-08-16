#version 400 core

layout(location = 1) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_matModelView;
uniform mat4 u_matProjection;

out fsInput {
	vec3 texCoord;
	vec3 position; //in view-space
} out_data;

void main()
{
	vec4 viewSpacePos = u_matModelView * vec4(a_Position, 1.0);

	out_data.texCoord = vec3(a_TexCoord, 0);
	out_data.position = viewSpacePos.xyz;

	gl_Position = u_matProjection * viewSpacePos;
}
