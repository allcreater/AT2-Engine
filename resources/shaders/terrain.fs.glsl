#version 410 core

uniform sampler3D u_texNoise;
uniform sampler2D u_texHeight;
uniform sampler2D u_texGrass, u_texRock;

in tesResult {
	vec2 texCoord;
	vec3 position; //in view-space
} input;

out vec4 FragColor;

void main()
{
	FragColor.rgb = texture(u_texHeight, input.texCoord).rrr;
	FragColor.a = 1.0;
}