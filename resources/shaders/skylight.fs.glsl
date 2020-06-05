#version 420 core

#define M_PI 3.1415926535897932384626433832795

precision mediump float;

in vec2 v_texCoord;

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
    double u_time;
};
//uniform mat4 u_matModel;
//uniform mat3 u_matNormal;

layout(binding = 2) uniform LightingBlock
{
	vec4 u_lightPos; //in view space
	float u_lightRadius;
	vec3 u_lightColor;
};

uniform sampler3D u_texNoise;
uniform sampler2D u_environmentMap;

uniform sampler2D u_colorMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_roughnessMetallicMap;
uniform sampler2D u_depthMap;

uniform vec3 u_sunDirection = vec3(0.0, 0.707, 0.707);

layout (location = 0) out vec4 FragColor;

vec4 getReflection(vec3 dirVS, float lod) // SphereMap
{
	vec3 dir = mat3(u_matInverseView) * dirVS;
	vec2 uv = vec2(atan(dir.x, dir.z)/M_PI/2, asin(dir.y)/M_PI+0.5);
	return textureLod(u_environmentMap, uv, lod);//vec4(uv*0.5+0.5, 0, 1);//
}


vec3 getFragPos(in vec3 screenCoord) 
{
    vec4 pos = u_matInverseProjection * vec4(screenCoord*2.0-1.0, 1.0);
    return pos.xyz/pos.w;
}

vec3 computeLighting (
		in vec3 lightVec, in float lightRadius, in vec3 lightColor,
 		in vec3 normal, in vec3 viewDir,
		in float roughtness, in vec3 F0,
		in vec3 diffuseColor
 	);

vec3 computeIBL (
	in int numSamples,
	in vec3 normal, in vec3 viewDir,
	in float roughtness, in vec3 F0
);

void main()
{
	vec2 texCoord = gl_FragCoord.xy / textureSize(u_colorMap, 0);

	float z = texture (u_depthMap, texCoord).r;
	vec3 fragPos = getFragPos(vec3(texCoord, z));

	vec4 color = texture (u_colorMap, texCoord);
	vec3 normal = texture (u_normalMap, texCoord).rgb;


	vec2 roughnessMetallic = texture (u_roughnessMetallicMap, texCoord).rg;
	vec3 F0 = vec3(0.05);
	float roughness = roughnessMetallic.r;

	vec3 lighting = computeLighting(mat3(u_matView) * u_sunDirection, 0.0, u_lightColor, normal, normalize(-fragPos), roughness, F0, color.rgb)*0.3;
	lighting += computeIBL(20, normal, normalize(-fragPos), roughness, F0);
	lighting *= 0.1;
	
	if (color.a < 0.5)
		FragColor = getReflection(-normalize(fragPos), 0)*2.0;
	else
		FragColor = vec4(lighting, 1.0);

	//FragColor = vec4(normal.xyz, 1);
}
