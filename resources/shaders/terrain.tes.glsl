#version 410 core

layout(quads, equal_spacing, ccw) in;

uniform CameraBlock
{
	mat4 u_matMW, u_matInverseMW, u_matProj, u_matInverseProj;
};

uniform float u_scaleH, u_scaleV;
uniform sampler2D u_texHeight;
uniform sampler2D u_texNoise;

in	tcsResult {
	vec2 texCoord;
} input[];

out tesResult {
	vec2 texCoord;
	vec3 position; //in view-space
} output;


#define Interpolate( a, p ) \
	( mix( \
		mix( a[0] p, a[1] p, gl_TessCoord.x ), \
		mix( a[3] p, a[2] p, gl_TessCoord.x ), \
		gl_TessCoord.y ) )


float getHeight (vec2 texCoord)
{
	return textureLod(u_texHeight, texCoord, 0.0).r * u_scaleV + texture(u_texNoise, texCoord*10.0).r * 3.0;
}

void main()
{
	vec4	worldPos 	= Interpolate( gl_in, .gl_Position );
	vec2	texCoord 	= Interpolate( input, .texCoord );

	worldPos.y += getHeight(texCoord);

	output.texCoord = texCoord;
	output.position = vec3(u_matMW * worldPos);
	gl_Position = u_matProj * u_matMW * worldPos;
}