#version 410 core

uniform sampler3D u_texNoise;
uniform sampler2D u_texHeight, u_texNormalMap;
uniform sampler2D u_texGrass, u_texRock;

in tesResult {
	vec2 texCoord;
	vec3 position; //in view-space
} input;

out vec4 FragColor;

void main()
{
	FragColor.rgb = texture(u_texNormalMap, input.texCoord).bgr*0.5 + texture(u_texGrass, input.texCoord*50.0).rgb*0.5;
	FragColor.a = 1.0;
}