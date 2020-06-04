#version 420 core

layout(location = 1) in vec3 a_Position;
layout(location = 2) in vec3 a_TexCoord;
layout(location = 3) in vec3 a_Normal;

layout (binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
};
uniform mat4 u_matModel;
uniform mat3 u_matNormal;

out fsInput {
	vec3 texCoord;
	vec3 normal;
	vec3 position; //in view-space
} output;

void main()
{

	vec4 viewSpacePos = u_matView * u_matModel * vec4(a_Position, 1.0);

	output.texCoord = a_TexCoord;
	output.normal = u_matNormal * a_Normal;
	output.position = viewSpacePos.xyz;

	gl_Position = u_matProjection * viewSpacePos;
}
