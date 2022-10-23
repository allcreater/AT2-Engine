#include <metal_stdlib>
using namespace metal;

struct VertexIn
{
    float verticalPosition [[ attribute(0) ]];
};

struct VertexUniforms
{
	vec2 boundsX
	uint numberOfPoints;
    float4x4 matProjection;
};

struct FragmentUniforms
{
	float4 color;
};

struct FragmentIn
{
    float4 position [[ position ]];
};


vertex FragmentIn vertex_main (
    const VertexIn vertex_in        [[stage_in]],
	constant VertexUniforms& params [[buffer(0)]],
	uint vertexId 					[[vertex_id]] )
{
	const float x = mix(u_BoundsX.x, u_BoundsX.y, float(vertexId)/float(params.numberOfPoints));
	const auto pos = float4(x, vertex_in.verticalPosition, 0.0, 1.0);

	return {
		params.matProjection * pos,
	};
}


fragment float4 fragment_main (
    const FragmentIn input [[stage_in]],
	constant FragmentUniforms& params [[buffer(0)]] )
{
	return params.color;
}