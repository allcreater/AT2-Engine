#version 420 core

layout(quads, equal_spacing, ccw) in;

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
	mat3 u_matNormal;
};

uniform float u_scaleH, u_scaleV;
uniform sampler2D u_texHeight;
uniform sampler2D u_texNoise;

in	tcsResult {
	vec2 texCoord;
} input[];

out tesResult {
	vec2 texCoord;
	float elevation;
	vec3 position; //in view-space
} output;


#define Interpolate( a, p ) \
	( mix( \
		mix( a[0] p, a[1] p, gl_TessCoord.x ), \
		mix( a[3] p, a[2] p, gl_TessCoord.x ), \
		gl_TessCoord.y ) )


float getHeight (in vec2 texCoord)
{
	return (textureLod(u_texHeight, texCoord, 0.0).r + textureLod(u_texHeight, texCoord, 4.0).r*0.00390625*2.0) * u_scaleV;// + texture(u_texNoise, texCoord*10.0).r * 3.0;
}

void main()
{
	vec4	worldPos 	= Interpolate( gl_in, .gl_Position );
	vec2	texCoord 	= Interpolate( input, .texCoord );

	float height = getHeight(texCoord);
	worldPos.y += height;


	output.texCoord = texCoord;
	output.elevation = height/u_scaleV;
	output.position = vec3(u_matView * worldPos);
	gl_Position = u_matProjection * u_matView * worldPos;
}