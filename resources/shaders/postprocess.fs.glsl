#version 420 core

precision mediump float;

in vec2 v_texCoord;

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
    double u_time;
};

uniform sampler3D u_texNoise;
uniform sampler2D u_colorMap;
uniform sampler2D u_depthMap;

uniform float u_tmExposure = 1.0;

layout (location = 0) out vec4 FragColor;

vec4 toneMapping (vec4 color)
{
	vec3 mapped_color = vec3(1.0) - exp(-color.rgb * u_tmExposure);

	return vec4(mapped_color, color.a); 
}

vec3 getEyeDir()
{
    vec4 device_normal = vec4(v_texCoord*2.0-1.0, 0.0, 1.0);
    vec3 eye_normal = normalize((u_matInverseProjection * device_normal).xyz);
    vec3 world_normal = normalize(mat3(u_matInverseView) * eye_normal);
    
    return world_normal;
}

vec3 getFragPos(in float z)
{
    vec4 pos = u_matInverseProjection * vec4(v_texCoord*2.0-1.0, z*2.0-1.0, 1.0);
    return pos.xyz/pos.w;
}

void main()
{
	vec4 color = texture(u_colorMap, v_texCoord);
	FragColor = toneMapping(color);

	//FragColor *= vec4(sin(texture(u_depthMap, v_texCoord).rrr), 1.0);

/*
	if (FragColor.r > 1.0 || FragColor.g > 1.0 || FragColor.b > 1.0)
		FragColor.rgb = vec3(1.0, 0.0, 0.0);
		*/
}
