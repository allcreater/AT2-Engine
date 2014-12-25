#version 400 core

precision mediump float;

in vec2 v_texCoord;

uniform CameraBlock
{
	mat4 u_matModelView, u_matInverseModelView, u_matProjection, u_matInverseProjection, u_matModelViewProjection;
	mat3 u_matNormal;
};

uniform float u_phase;
uniform sampler3D u_texNoise;
uniform sampler2D u_colorMap;
uniform sampler2D u_depthMap;

layout (location = 0) out vec4 FragColor;


vec3 getEyeDir()
{
    vec4 device_normal = vec4(v_texCoord*2.0-1.0, 0.0, 1.0);
    vec3 eye_normal = normalize((u_matInverseProjection * device_normal).xyz);
    vec3 world_normal = normalize(mat3(u_matInverseModelView) * eye_normal);
    
    return world_normal;
}

vec3 getFragPos(in float z)
{
    vec4 pos = u_matInverseProjection * vec4(v_texCoord*2.0-1.0, z*2.0-1.0, 1.0);
    return pos.xyz/pos.w;
}

void main()
{
	FragColor = texture(u_colorMap, v_texCoord);

/*
	if (FragColor.r > 1.0 || FragColor.g > 1.0 || FragColor.b > 1.0)
		FragColor.rgb = vec3(1.0, 0.0, 0.0);
		*/
}
