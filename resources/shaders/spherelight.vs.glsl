#version 400 core

layout(location = 1) in vec3 a_Normal;

uniform CameraBlock
{
	mat4 u_matModelView, u_matInverseModelView, u_matProjection, u_matInverseProjection, u_matModelViewProjection;
	mat3 u_matNormal;
};

uniform LightingBlock
{
	vec4 u_lightPos; //in view space
	float u_lightRadius;
	vec3 u_lightColor;
};

out vec3 v_pos;
out vec2 v_texCoord;

void main()
{
	vec4 pos = u_matModelView * (u_lightPos + vec4(a_Normal, 0.0) * u_lightRadius); //in view space
	gl_Position = u_matProjection * pos;
	
	v_texCoord = (gl_Position.xy/gl_Position.w)*0.5 + 0.5;

	v_pos = pos.xyz;
}
