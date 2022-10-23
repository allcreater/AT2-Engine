#include <metal_stdlib>
using namespace metal;

struct VertexIn
{
    float3 position [[ attribute(1) ]];
};

struct VertexUniforms
{
};


struct FragmentIn
{
    float4 position [[ position ]];
    float2 texCoord;
};


vertex FragmentIn vertex_main (
    const VertexIn vertex_in        [[stage_in]],
	constant VertexUniforms& params [[buffer(0)]] )
{
	return {
		float4(vertex_in.position, 1.0),
		vertex_in.position.xy*0.5 + float2(0.5)
	};
}


namespace
{

//it could be approximated or taken from texture1D
float gaussian(float x, float mean, float dispersion)
{
	const float a = 1.0/(sqrt(2 * 3.1415));
	return a/dispersion * exp(-(x - mean)*(x - mean)/(2.0 * dispersion * dispersion));
}

float4 getBlurredBackground(texture2d<float, access::sample> background, float2 texCoord, float2 blurDirection)
{
	constexpr sampler sampler(min_filter::linear, mag_filter::linear, mip_filter::linear, address::repeat);
	constexpr int numOfPoints = 20;
	const float2 offset = blurDirection/float2(background.get_width(), background.get_height());

	float4 resultColor{0.0, 0.0, 0.0, 0.0};
	for (int i = -numOfPoints; i <= numOfPoints; ++i)
	{
		resultColor += background.sample(sampler, texCoord + offset * i) * gaussian(i, 0, numOfPoints/2.0);
	}

	return resultColor;
}

}

struct FragmentUniforms
{
	float4 u_ScreenAABB;
	float4 u_Color;
	float2 u_BlurDirection;
	float2 u_BorderThickness;
};

fragment float4 fragment_main (
    const FragmentIn input [[stage_in]],
	constant FragmentUniforms& params [[buffer(0)]],
	texture2d<float, access::sample> u_BackgroundTexture [[texture(0)]])
{
	const float2 minDist = min(abs(input.position.xy - params.u_ScreenAABB.xy), abs(input.position.xy - params.u_ScreenAABB.zw));

	const float4 color = getBlurredBackground(u_BackgroundTexture, input.texCoord, params.u_BlurDirection*0.7);

	if (minDist.x <= params.u_BorderThickness.x || minDist.y <= params.u_BorderThickness.y || (minDist.x + minDist.y) <= (params.u_BorderThickness.x + params.u_BorderThickness.y)*2.0)
		discard_fragment();

	return float4(mix(color.rgb, params.u_Color.rgb, params.u_Color.a*0.5), params.u_Color.a*2.0);
}