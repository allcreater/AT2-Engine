#version 420 core

uniform sampler2D u_texAlbedo;

in fsInput {
	vec3 texCoord;
	vec3 position; //in view-space
} in_data;

layout (location = 0) out vec4 FragColor;

void main()
{
	vec2 texCoord = in_data.texCoord.st;
    vec4 albedo = texture(u_texAlbedo, texCoord);

	FragColor = albedo;
}