#version 420 core

//per vertex
layout(location = 1) in vec3 a_Normal;
//per instance
layout(location = 2) in vec3 a_LightPos;
layout(location = 3) in vec3 a_LightInsensity;
layout(location = 4) in float a_LightRadius;


layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
    double u_time;
};
//uniform mat4 u_matModel;
//uniform mat3 u_matNormal;

out	vsResult {
	vec3 pos;
	vec2 texCoord;

	vec4 lightPos;
	vec3 lightIntensity;
	float lightRadius;
} vsOut;

void main()
{
	const vec4 lightPos = vec4(a_LightPos, 1.0);

	vec4 worldPos = u_matView * (lightPos + vec4(a_Normal, 0.0) * a_LightRadius); //in view space
	gl_Position = u_matProjection * worldPos;
	

	vsOut.pos = worldPos.xyz;
	vsOut.texCoord = (gl_Position.xy/gl_Position.w)*0.5 + 0.5;

	vsOut.lightPos = lightPos;
	vsOut.lightIntensity = a_LightInsensity;
	vsOut.lightRadius = a_LightRadius;
}
