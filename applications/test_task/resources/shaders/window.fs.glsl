#version 420 core

precision mediump float;

in vec2 v_texCoord;

layout (location = 0) out vec4 FragColor;

uniform vec4 u_ScreenAABB;
uniform vec4 u_Color;
uniform sampler2D u_BackgroundTexture;

uniform vec2 u_BlurDirection;
uniform vec2 u_BorderThickness = vec2(10.0, 10.0);

//it could be approximated or taken from texture1D
float gaussian(in float x, in float mean, in float dispersion)
{
	const float a = 1.0/(sqrt(2 * 3.1415));
	return a/dispersion * exp(-(x - mean)*(x - mean)/(2.0 * dispersion * dispersion));
}

vec4 getBlurredBackground(in vec2 texCoord, in vec2 blurDirection)
{
	const int numOfPoints = 20;
	vec2 offset = blurDirection/textureSize(u_BackgroundTexture, 0);

	vec4 resultColor = vec4(0.0);
	for (int i = -numOfPoints; i <= numOfPoints; ++i)
	{
		resultColor += texture(u_BackgroundTexture, texCoord + offset * i) * gaussian(i, 0, numOfPoints/2.0);
	}

	return resultColor;// /(numOfPoints*2 + 1.0);
}

void main()
{
	vec2 minDist = min(abs(gl_FragCoord.xy - u_ScreenAABB.xy), abs(gl_FragCoord.xy - u_ScreenAABB.zw));

	vec4 color = getBlurredBackground(v_texCoord, u_BlurDirection*0.7);
	FragColor = vec4(mix(color.rgb, u_Color.rgb, u_Color.a*0.5), u_Color.a*2.0);

	if (minDist.x <= u_BorderThickness.x || minDist.y <= u_BorderThickness.y || (minDist.x + minDist.y) <= (u_BorderThickness.x + u_BorderThickness.y)*2.0)
		//FragColor = vec4(1.0,0.0,0.0,1.0);
		discard;

	//FragColor = texture(u_BackgroundTexture, v_texCoord);

	//FragColor = vec4(gl_FragCoord.xy/vec2(1280, 800), 0.0, 1.0);
}