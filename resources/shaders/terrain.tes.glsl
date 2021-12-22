#version 420 core

layout(quads, equal_spacing, cw) in;

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
    double u_time;
};
uniform mat4 u_matModel;
uniform mat3 u_matNormal;

uniform sampler2D u_texHeight;
uniform sampler3D u_texNoise;


in	tcsResult {
	vec2 texCoord;
} in_data[];

out tesResult {
	vec2 texCoord;
	float elevation;
	vec3 position; //in view-space
	vec3 normal; //ws
} out_data;


#define Interpolate( a, p ) \
	( mix( \
		mix( a[0] p, a[1] p, gl_TessCoord.x ), \
		mix( a[3] p, a[2] p, gl_TessCoord.x ), \
		gl_TessCoord.y ) )


float getWaterHeight (in vec2 texCoord)
{
	float t = float(u_time);
	float result = -0.01;//texture(u_texNoise, vec3(texCoord*20.0, t * 0.01)).r * 0.005 - 0.01;

	const mat2 m = mat2(0.8, -0.6, 0.6, 0.8);
	for (int i = 0; i < 3; ++i)
	{
		vec4 noise = texture(u_texNoise, vec3(texCoord*5.0, t * 0.01));
		result += noise.r * 0.005 * pow(0.5, i);

		t += noise.g;
		texCoord = m * texCoord * 2.0;
	}

	return result * 0.2;
}

float getHeight (in vec2 texCoord)
{
	float result = texture(u_texHeight, texCoord).r;
	
	if (result < 0.001) //water effect
		return getWaterHeight(texCoord);

	return result;
}

uniform float step = 1.9/4096.0;

vec3 getNormal(in vec2 texCoord)
{
	//vec4 heights = textureGather(u_texHeight, texCoord);
	//return normalize(vec3(heights.w - heights.z, 0.0005, heights.w - heights.x));

	return normalize(vec3(
		getHeight(texCoord+vec2(-1,0)*step) - getHeight(texCoord+vec2(0,1)*step),
		0.0005, 
		getHeight(texCoord+vec2(0,-1)*step) - getHeight(texCoord+vec2(0,1)*step)
	));
}

void main()
{
	vec4	worldPos 	= Interpolate( gl_in, .gl_Position );
	vec2	texCoord 	= Interpolate( in_data, .texCoord );

	float height = getHeight(texCoord);
	worldPos.y += height;


	out_data.texCoord = texCoord;
	out_data.elevation = height;
	out_data.position = vec3(u_matView * worldPos);

	out_data.normal = u_matNormal * getNormal(texCoord);
	gl_Position = u_matProjection * u_matView * u_matModel * worldPos;
}