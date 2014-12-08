#version 400 core

precision mediump float;

uniform CameraBlock
{
	mat4 u_matMW, u_matInverseMW, u_matProj, u_matInverseProj;
};

uniform float u_phase;
uniform sampler3D u_texNoise;


in vec4 v_color;
in vec2 v_texCoord;

layout (location = 0) out vec4 FragColor;


vec3 getEyeDir()
{
    vec4 device_normal = vec4(v_texCoord*2.0-1.0, 0.0, 1.0);
    vec3 eye_normal = normalize((u_matInverseProj * device_normal).xyz);
    vec3 world_normal = normalize(mat3(u_matInverseMW) * eye_normal);
    
    return world_normal;
}


void main()
{
	vec3 dir = getEyeDir();

	FragColor = vec4(texture(u_texNoise, dir*0.1).rgb, 1.0);
}
