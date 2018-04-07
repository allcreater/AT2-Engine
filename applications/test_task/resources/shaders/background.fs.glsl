#version 420 core

precision mediump float;

in vec2 v_texCoord;

layout (location = 0) out vec4 FragColor;

uniform vec4 u_Color;
uniform sampler2D u_BackgroundTexture;

void main()
{
	vec2 texCoord = gl_FragCoord.xy/textureSize(u_BackgroundTexture, 0);
	vec4 color = texture(u_BackgroundTexture, texCoord);
	FragColor = u_Color * color;

	//FragColor = u_Color * vec4(vec3(0.3), 0.7);
}