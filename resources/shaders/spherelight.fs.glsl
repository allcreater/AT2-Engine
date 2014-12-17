#version 400 core

precision mediump float;

in vec3 v_pos;
in vec2 v_texCoord;

uniform CameraBlock
{
	mat4 u_matMW, u_matInverseMW, u_matProj, u_matInverseProj;
	mat3 u_matNormal;
};

uniform LightingBlock
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


vec3 getEyeDir()
{
    vec4 device_normal = vec4(v_texCoord*2.0-1.0, 0.0, 1.0);
    vec3 eye_normal = normalize((u_matInverseProj * device_normal).xyz);
    vec3 world_normal = normalize(mat3(u_matInverseMW) * eye_normal);
    
    return world_normal;
}

vec3 getFragPos(float z, vec2 screenCoord) 
{
    vec4 pos = u_matInverseProj * vec4(screenCoord*2.0-1.0, z*2.0-1.0, 1.0);
    return pos.xyz/pos.w;
}

void main()
{
	vec2 texCoord = gl_FragCoord.xy/1024.0;

	float z = texture (u_depthMap, texCoord);
	vec3 fragPos = getFragPos(z, texCoord);


	
	vec3 normal = texture(u_normalMap, texCoord).rgb;

	vec4 color = texture(u_colorMap, texCoord);


	float attenuation = 1.0 - clamp(length(fragPos - vec3(u_matMW * u_lightPos))/u_lightRadius, 0.0, 1.0);


	FragColor = vec4(color.rgb*attenuation, 1.0);
}
