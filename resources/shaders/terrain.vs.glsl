#version 410 core

layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_matMW, u_matProj;
uniform float u_scaleH, u_scaleV;

out	vsResult {
	vec2 texCoord;
} output;

void main()
{
	vec2 texCoord = a_TexCoord;

	output.texCoord = texCoord;
	gl_Position = vec4((texCoord.x-0.5)*u_scaleH, -u_scaleV*0.5, (texCoord.y-0.5)*u_scaleH, 1.0);
}
