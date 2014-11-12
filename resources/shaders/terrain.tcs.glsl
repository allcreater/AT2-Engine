#version 410 core

layout(vertices = 4) out;

uniform float	unMaxTessLevel	= 64.0;

in	vsResult {
	vec2 texCoord;
} input[];

out	tcsResult {
	vec2 texCoord;
} output[];

void main()
{
	if (gl_InvocationID == 0)
	{
		gl_TessLevelInner[0] = unMaxTessLevel;
		gl_TessLevelInner[1] = unMaxTessLevel;

		gl_TessLevelOuter[0] = unMaxTessLevel;
		gl_TessLevelOuter[1] = unMaxTessLevel;
		gl_TessLevelOuter[2] = unMaxTessLevel;
		gl_TessLevelOuter[3] = unMaxTessLevel;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	output[gl_InvocationID].texCoord = input[gl_InvocationID].texCoord;
}