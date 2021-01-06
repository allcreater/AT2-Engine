#version 420 core

precision mediump float;

in	vsResult {
	vec3 pos;
	vec2 texCoord;

	vec4 lightPos;
	vec3 lightIntensity;
	float lightRadius;
} fsIn;

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
    double u_time;
};
//uniform mat4 u_matModel;
//uniform mat3 u_matNormal;

// layout(binding = 2) uniform LightingBlock
// {
// 	vec4 u_lightPos; //in view space
// 	float u_lightRadius;
// 	vec3 u_lightColor;
// };

uniform sampler3D u_texNoise;

uniform sampler2D u_colorMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_roughnessMetallicMap;
uniform sampler2D u_depthMap;

layout (location = 0) out vec4 FragColor;


vec3 computeLighting (
		in vec3 lightVec, in float lightRadius, in vec3 lightColor,
 		in vec3 normal, in vec3 viewDir,
		in float roughtness, in vec3 F0,
		in vec3 diffuseColor
 	);

vec3 getFragPos(in vec3 screenCoord) 
{
    vec4 pos = u_matInverseProjection * vec4(screenCoord*2.0-1.0, 1.0);
    return pos.xyz/pos.w;
}

void main()
{
	vec2 texCoord = gl_FragCoord.xy / textureSize(u_colorMap, 0);;

	float z = texture (u_depthMap, texCoord).r;
	vec3 fragPos = getFragPos(vec3(texCoord, z));


	vec3 lightVector = (u_matView * fsIn.lightPos).xyz - fragPos;
	vec3 normal = texture(u_normalMap, texCoord).rgb;
	
	vec2 roughnessMetallic = texture (u_roughnessMetallicMap, texCoord).rg;



	vec4 color = texture(u_colorMap, texCoord);
	vec3 F0 = mix(vec3(0.05), color.rgb, roughnessMetallic.g);
	float roughness = roughnessMetallic.r;

	vec3 lighting = computeLighting(lightVector, fsIn.lightRadius, fsIn.lightIntensity, normal, normalize(-fragPos), roughness, F0, color.rgb);
	

	FragColor = vec4(lighting, 1.0);
	//FragColor = vec4(normal, 1.0);
}
