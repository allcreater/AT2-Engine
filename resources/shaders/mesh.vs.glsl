#version 420 core

layout(location = 1) in vec3 a_Position;
layout(location = 2) in vec3 a_TexCoord;
layout(location = 3) in vec3 a_Normal;

layout(location = 4) in uvec4 a_Joints;
layout(location = 5) in vec4 a_Weights;

uniform bool u_useSkinning = false;
uniform mat4 u_skeletonMatrices[100];

layout (binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
	double u_time;
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
	mat4 modelView = u_matView * u_matModel;
	if (u_useSkinning)
	{
		modelView = modelView * (
			a_Weights.x * u_skeletonMatrices[int(a_Joints.x)] +
			a_Weights.y * u_skeletonMatrices[int(a_Joints.y)] +
			a_Weights.z * u_skeletonMatrices[int(a_Joints.z)] +
			a_Weights.w * u_skeletonMatrices[int(a_Joints.w)]);
	}
	vec4 viewSpacePos = modelView * vec4(a_Position, 1.0);


	output.texCoord = a_TexCoord;
	output.normal = u_matNormal * a_Normal;
	output.position = viewSpacePos.xyz;

	gl_Position = u_matProjection * viewSpacePos;
}
