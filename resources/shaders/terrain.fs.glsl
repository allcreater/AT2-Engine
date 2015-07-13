#version 420 core

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
	mat3 u_matNormal;
};

uniform sampler3D u_texNoise;
uniform sampler2D u_texHeight, u_texNormalMap;
uniform sampler2D u_texGrass, u_texRock;

in tesResult {
	vec2 texCoord;
	float elevation;
	vec3 position; //in view-space
} input;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragNormal;

//not using there
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
	vec3 normalFromMap = (texture(u_texNormalMap, input.texCoord).rbg * 2.0 - 1.0)*vec3(1.0, 1.0, -1.0);

	vec2 texCoord = input.texCoord*200.0;
	FragColor.rgb = mix(texture(u_texGrass, texCoord).rgb, texture(u_texRock, texCoord*2.0).rgb, smoothstep(0.1, 0.5, length(normalFromMap.xz)));

	if (input.elevation <= 0.01)
		FragColor.rgb = vec3(0.2, 0.3, 1.0);

	FragColor.rgb = mix(FragColor.rgb, vec3(1.0), smoothstep(0.31, 0.35, input.elevation * (-length(normalFromMap.xz)+0.95)));

	FragColor.a = 1.0;

	vec3 normal = u_matNormal * normalFromMap;
	FragNormal = vec4(normal, 1.0);
}