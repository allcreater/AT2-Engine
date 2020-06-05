#version 420 core

layout(location = 1) in vec2 a_TexCoord;

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
    double u_time;
};
uniform mat4 u_matModel;
uniform mat3 u_matNormal;

out	vsResult {
	vec2 texCoord;
} output;

void main()
{
	vec2 texCoord = a_TexCoord;

	output.texCoord = texCoord;
	gl_Position = vec4((texCoord.x-0.5), -0.5, (texCoord.y-0.5), 1.0);
}
