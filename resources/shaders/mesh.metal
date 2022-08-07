#include <metal_stdlib>
#include <metal_matrix>
using namespace metal;

struct VertexIn
{
    float3 position         [[ attribute(1) ]];
    float3 texCoord         [[ attribute(2) ]];
    float3 normal           [[ attribute(3) ]];
    uint4  skinning_joints  [[ attribute(4) ]];
    float4 skinning_weights [[ attribute(5) ]];
};

struct CameraBlock
{
	float4x4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
	float u_time;
};


struct VertexUniforms
{
    constexpr static constant uint kMaxBones = 200;
    float4x4 u_matModel;
    float3x3 u_matNormal;
    bool u_useSkinning;
    //float4x4 u_skeletonMatrices[kMaxBones];
};

//TODO: use special buffer
struct Bone
{
    float4x4 model;
    float3x3 normal;
};

struct FragmentIn
{
    float4 position [[ position ]];
    float3 texCoord;
    float3 normal;
    float3 view_space_position;
};

vertex FragmentIn vertex_main (
    const VertexIn vertex_in        [[stage_in]],
    constant VertexUniforms& params [[buffer(0)]],
    constant CameraBlock& CameraBlock    [[buffer(6)]] /*,
    constant Bone* bones            [[buffer(2)]] */)
{
	float3x3 normalMatrix = params.u_matNormal;
	float4x4 modelView = CameraBlock.u_matView * params.u_matModel;
	if (params.u_useSkinning)
	{
		// modelView = modelView * (
		// 	vertex_in.skinning_weights.x * params.u_skeletonMatrices[int(vertex_in.skinning_joints.x)] +
		// 	vertex_in.skinning_weights.y * params.u_skeletonMatrices[int(vertex_in.skinning_joints.y)] +
		// 	vertex_in.skinning_weights.z * params.u_skeletonMatrices[int(vertex_in.skinning_joints.z)] +
		// 	vertex_in.skinning_weights.w * params.u_skeletonMatrices[int(vertex_in.skinning_joints.w)]);

		// normalMatrix = normalMatrix * (
		// 	vertex_in.skinning_weights.x * bones[int(vertex_in.skinning_joints.x)].normal +
		// 	vertex_in.skinning_weights.y * bones[int(vertex_in.skinning_joints.y)].normal +
		// 	vertex_in.skinning_weights.z * bones[int(vertex_in.skinning_joints.z)].normal +
		// 	vertex_in.skinning_weights.w * bones[int(vertex_in.skinning_joints.w)].normal);
	}
	const float4 viewSpacePos = modelView * float4(vertex_in.position, 1.0);

    return { 
        CameraBlock.u_matProjection * viewSpacePos,
        vertex_in.texCoord,
        normalMatrix * vertex_in.normal,
        viewSpacePos.xyz
    };
}

//TODO: deferred version + use PBR lighting model
fragment float4 fragment_main (
    const FragmentIn input                                      [[stage_in]], 
    texture2d<float, access::sample> u_texAlbedo                [[texture(0)]],
    texture2d<float, access::sample> u_texNormalMap             [[texture(1)]],
    texture2d<float, access::sample> u_texAoRoughnessMetallic   [[texture(2)]] )
{
    constexpr sampler sampler(min_filter::linear, mag_filter::linear, mip_filter::linear, address::repeat);

    const auto color = u_texAlbedo.sample(sampler, input.texCoord.xy);
    return color;
}
