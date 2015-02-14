#version 410 core

layout(vertices = 4) out;

uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
	mat3 u_matNormal;
};

uniform float	unMaxTessLevel	= 64.0;

in	vsResult {
	vec2 texCoord;
} input[];

out	tcsResult {
	vec2 texCoord;
} output[];

bool PointInFrustum(in vec4 point)
{
	vec4 pointProj = u_matViewProjection * point;
	float limit = pointProj.w + 50.0;

	return !((pointProj.x < -limit) || (pointProj.x > limit));// || (pointProj.y < -limit) || (pointProj.y > limit));
}


void main()
{
	if (gl_InvocationID == 0)
	{
		float level = (PointInFrustum(gl_in[0].gl_Position) || PointInFrustum(gl_in[1].gl_Position) || PointInFrustum(gl_in[2].gl_Position) || PointInFrustum(gl_in[3].gl_Position)) ? unMaxTessLevel : 2.0;
		gl_TessLevelInner[0] = level;
		gl_TessLevelInner[1] = level;

		gl_TessLevelOuter[0] = level;
		gl_TessLevelOuter[1] = level;
		gl_TessLevelOuter[2] = level;
		gl_TessLevelOuter[3] = level;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	output[gl_InvocationID].texCoord = input[gl_InvocationID].texCoord;
}