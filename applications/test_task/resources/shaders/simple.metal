#include <metal_stdlib>
using namespace metal;

struct VertexIn
{
    float2 position [[ attribute(0) ]];
    float4 color 	[[ attribute(1) ]];
};

struct VertexUniforms
{
    float4x4 matProjection;
};

struct FragmentIn
{
    float4 position [[ position ]];
    float4 color;
};


vertex FragmentIn vertex_main (
    const VertexIn vertex_in        [[stage_in]],
	constant VertexUniforms& params [[buffer(0)]] )
{
	return {
		params.matProjection * float4(vertex_in.position, 0.0, 1.0),
		vertex_in.color
	};
}


fragment float4 fragment_main (
    const FragmentIn input [[stage_in]] )
{
	return input.color;
}