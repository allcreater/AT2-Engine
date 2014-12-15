#version 400 core

layout(location = 1) in vec3 a_Normal;

uniform CameraBlock
{
	mat4 u_matMW, u_matInverseMW, u_matProj, u_matInverseProj;
	mat3 u_matNormal;
};

uniform LightingBlock
{
	vec3 u_lightPos; //in view space
	float u_lightRadius;
};

out vec3 v_pos;
out vec2 v_texCoord;

void main()
{
	vec3 pos = u_lightPos + vec3(u_matMW) * (a_Normal * 100.0); //in view space
	gl_Position = u_matProj * u_matMW * vec4(a_Normal * 100.0, 1.0);//u_matProj * vec4 (pos, 1.0);
	
	v_texCoord = gl_Position.xy;

	v_pos = pos;
}
