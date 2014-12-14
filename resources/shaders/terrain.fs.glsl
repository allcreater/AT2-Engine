#version 410 core

uniform CameraBlock
{
	mat4 u_matMW, u_matInverseMW, u_matProj, u_matInverseProj;
	mat3 u_matNormal;
};

uniform sampler3D u_texNoise;
uniform sampler2D u_texHeight, u_texNormalMap;
uniform sampler2D u_texGrass, u_texRock;

in tesResult {
	vec2 texCoord;
	vec3 position; //in view-space
} input;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragNormal;

//not using there
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

void main()
{
	FragColor.rgb = texture(u_texNormalMap, input.texCoord).bgr*0.5 + texture(u_texGrass, input.texCoord*50.0).rgb*0.5;
	FragColor.a = 1.0;

	vec3 normal = mat3(u_matMW) * (texture(u_texNormalMap, input.texCoord).xyz*2.0 - 1.0);
	FragNormal = vec4(normal, 1.0);
}