#version 420 core

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
};
uniform mat4 u_matModel;
uniform mat3 u_matNormal;


uniform sampler2D u_texNormalMap;
uniform sampler2D u_texAlbedo;

in fsInput {
	vec3 texCoord;
	vec3 normal;
	vec3 position; //in view-space
} input;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragNormal;

mat3 cotangent_frame(in vec3 normal, in vec3 pos, in vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( pos );
    vec3 dp2 = dFdy( pos );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, normal );
    vec3 dp1perp = cross( normal, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, normal );
}

void main()
{
	//vec3 normalFromMap = (texture(u_texNormalMap, input.texCoord).rbg * 2.0 - 1.0)*vec3(1.0, 1.0, -1.0);

	vec2 texCoord = input.texCoord.st;

	FragColor.rgba = texture (u_texAlbedo, texCoord.st);

	vec3 normal = input.normal;
	FragNormal = vec4(normal, 1.0);
}