#version 420 core

precision mediump float;

in vec2 v_texCoord;

layout (location = 0) out vec4 FragColor;

uniform vec4 u_Color;
uniform sampler2D u_BackgroundTexture;
uniform vec2 u_BlurDirection = vec2(1.0, 0.0);

vec4 getBlurredBackground(in vec2 texCoord, in vec2 blurDirection)
{
	vec2 offset = blurDirection/textureSize(u_BackgroundTexture, 0);

	vec4 resultColor = vec4(0.0);
	for (int i = -5; i <= 5; ++i)
	{
		resultColor += texture(u_BackgroundTexture, texCoord + offset * i);
	}

	return resultColor/11.0;
}

void main()
{
	vec4 color = getBlurredBackground(v_texCoord, u_BlurDirection);
	FragColor = color * vec4(0.7);
}