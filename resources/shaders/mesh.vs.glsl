#version 410 core

layout(location = 1) in vec3 a_Position;
layout(location = 2) in vec3 a_TexCoord;
layout(location = 3) in vec3 a_Normal;

uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
	mat3 u_matNormal;
};

uniform mat4 u_matModel;

out fsInput {
	vec2 texCoord;
	vec3 normal;
	vec3 position; //in view-space
} output;

void main()
{

	vec4 viewSpacePos = u_matView * u_matModel * vec4(a_Position, 1.0);

	output.texCoord = a_TexCoord.st;
	output.normal = a_Normal;
	output.position = viewSpacePos.xyz;

	gl_Position = u_matProjection * viewSpacePos;
}
