#include <metal_stdlib>
using namespace metal;

struct VertexIn
{
    float3 position [[ attribute(1) ]];
    float2 texCoord [[ attribute(2) ]];
};

struct VertexUniforms
{
    float4x4 u_matModelView;
    float4x4 u_matProjection;
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
    const auto viewSpacePos = params.u_matModelView * float4(vertex_in.position, 1);
    return { 
        params.u_matProjection * viewSpacePos,
        vertex_in.texCoord
    };
}

fragment float4 fragment_main (
    const FragmentIn input                      [[stage_in]], 
    texture2d<float, access::sample> u_texAlbedo  [[texture(0)]] )
{
    constexpr sampler sampler(min_filter::linear, mag_filter::linear, mip_filter::linear, address::repeat);

    const auto color = u_texAlbedo.sample(sampler, input.texCoord);
    return color;
}
