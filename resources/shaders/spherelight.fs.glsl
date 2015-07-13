#version 420 core

precision mediump float;

in vec3 v_pos;
in vec2 v_texCoord;

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
	mat3 u_matNormal;
};

layout(binding = 2) uniform LightingBlock
{
	vec4 u_lightPos; //in view space
	float u_lightRadius;
	vec3 u_lightColor;
};

uniform sampler3D u_texNoise;
uniform sampler2D u_colorMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_depthMap;

layout (location = 0) out vec4 FragColor;


vec3 getFragPos(in vec3 screenCoord) 
{
    vec4 pos = u_matInverseProjection * vec4(screenCoord*2.0-1.0, 1.0);
    return pos.xyz/pos.w;
}

void main()
{
	vec2 texCoord = gl_FragCoord.xy/1024.0;

	float z = texture (u_depthMap, texCoord).r;
	vec3 fragPos = getFragPos(vec3(texCoord, z));


	vec3 lightDir = fragPos - (u_matView * u_lightPos).xyz;
	float lightDirLength = length(lightDir);
	lightDir = lightDir / lightDirLength; //normalize dir vector

	vec3 normal = texture(u_normalMap, texCoord).rgb;

	float Kd = max(dot (normal, -lightDir), 0.0);


	float attenuation = 1.0 - clamp(lightDirLength/u_lightRadius, 0.0, 1.0);
	
	vec4 color = texture(u_colorMap, texCoord)*Kd;

	FragColor = vec4(color.rgb*attenuation*attenuation, 1.0);
}
