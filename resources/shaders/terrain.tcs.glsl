#version 420 core

layout(vertices = 4) out;

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
    double u_time;
};
uniform mat4 u_matModel;
uniform mat3 u_matNormal;

uniform sampler2D u_texHeight;

uniform float unMaxTessLevel	= 64.0;

in	vsResult {
	vec2 texCoord;
} in_data[];

out	tcsResult {
	vec2 texCoord;
} out_data[];

bool PointInFrustum(in vec4 point)
{
	vec4 pointProj = u_matViewProjection * u_matModel * point;
	float limit = pointProj.w;

	return abs(pointProj.x) < limit && abs(pointProj.y) < limit;
}
//TODO: don't duplicate code - use include or something like that
float getHeight (in vec2 texCoord)
{
	float result = texture(u_texHeight, texCoord).r;
	
	if (result < 0.001) //water
		result = 0.0;

	return result;
}

bool PointInFrustum(int i)
{
	vec4 pos = gl_in[i].gl_Position;
	pos.y += getHeight(in_data[i].texCoord);
	return PointInFrustum(pos);
}

void main()
{
	if (gl_InvocationID == 0)
	{
		const bool inFrustum = PointInFrustum(0) || PointInFrustum(1) || PointInFrustum(2) || PointInFrustum(3);
		float level = inFrustum ? unMaxTessLevel : 2.0;
		//float level = (PointInFrustum(gl_in[0].gl_Position) || PointInFrustum(gl_in[1].gl_Position) || PointInFrustum(gl_in[2].gl_Position) || PointInFrustum(gl_in[3].gl_Position)) ? unMaxTessLevel : 2.0;
		gl_TessLevelInner[0] = level;
		gl_TessLevelInner[1] = level;

		gl_TessLevelOuter[0] = level;
		gl_TessLevelOuter[1] = level;
		gl_TessLevelOuter[2] = level;
		gl_TessLevelOuter[3] = level;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	out_data[gl_InvocationID].texCoord = in_data[gl_InvocationID].texCoord;
}