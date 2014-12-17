#version 400 core

precision mediump float;

in vec2 v_texCoord;

uniform CameraBlock
{
	mat4 u_matMW, u_matInverseMW, u_matProj, u_matInverseProj;
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
    vec3 eye_normal = normalize((u_matInverseProj * device_normal).xyz);
    vec3 world_normal = normalize(mat3(u_matInverseMW) * eye_normal);
    
    return world_normal;
}

vec3 getFragPos(float z)
{
    vec4 pos = u_matInverseProj * vec4(v_texCoord*2.0-1.0, z*2.0-1.0, 1.0);
    return pos.xyz/pos.w;
}

void main()
{
	FragColor = texture(u_colorMap, v_texCoord);
}
