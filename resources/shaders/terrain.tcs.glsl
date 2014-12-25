#version 410 core

layout(vertices = 4) out;

uniform CameraBlock
{
	mat4 u_matModelView, u_matInverseModelView, u_matProjection, u_matInverseProjection, u_matModelViewProjection;
	mat3 u_matNormal;
};

uniform float	unMaxTessLevel	= 64.0;

in	vsResult {
	vec2 texCoord;
} input[];

out	tcsResult {
	vec2 texCoord;
} output[];


vec4 clippingPlane[6] = vec4[6] (
	//right and left
	normalize(u_matModelViewProjection[3] - u_matModelViewProjection[0]),
	normalize(u_matModelViewProjection[3] + u_matModelViewProjection[0]),
	//down and up
	normalize(u_matModelViewProjection[3] - u_matModelViewProjection[1]),
	normalize(u_matModelViewProjection[3] + u_matModelViewProjection[1]),
	//far and near
	normalize(u_matModelViewProjection[3] - u_matModelViewProjection[2]),
	normalize(u_matModelViewProjection[3] + u_matModelViewProjection[2])
	);

bool PointInFrustum(vec3 point)
{
	vec3 dir = point - u_matInverseModelView[3].xyz;
	return dot(dir, -u_matInverseModelView[2].xyz) > 0;
	/*
	return (dot(clippingPlane[0].xyz, point) > -clippingPlane[0].w)
		&& (dot(clippingPlane[1].xyz, point) > -clippingPlane[1].w)
		&& (dot(clippingPlane[2].xyz, point) > -clippingPlane[2].w)
		&& (dot(clippingPlane[3].xyz, point) > -clippingPlane[3].w)
		&& (dot(clippingPlane[4].xyz, point) > -clippingPlane[4].w)
		&& (dot(clippingPlane[5].xyz, point) > -clippingPlane[5].w);*/
}

void main()
{
	if (gl_InvocationID == 0)
	{
		float level = (PointInFrustum(gl_in[0].gl_Position.xyz) || PointInFrustum(gl_in[1].gl_Position.xyz) || PointInFrustum(gl_in[2].gl_Position.xyz) || PointInFrustum(gl_in[3].gl_Position.xyz)) ? unMaxTessLevel : 2.0;
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