#version 420 core

layout(binding = 1) uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
    double u_time;
};
uniform mat4 u_matModel;
uniform mat3 u_matNormal;

uniform sampler3D u_texNoise;
uniform sampler2D u_texHeight, u_texNormalMap;
uniform sampler2D u_texGrass, u_texRock;

in tesResult {
	vec2 texCoord;
	float elevation;
	vec3 position; //in view-space
	vec3 normal; //ws
} in_data;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragNormal;
layout (location = 2) out vec4 RoughnessMetallic; 

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

//input: screen coord is in range -1..1
//output: position in eye space
vec3 getFragPos(in vec3 screenCoord)
{
    vec4 pos = u_matInverseProjection * vec4(screenCoord, 1.0);
    return pos.xyz/pos.w;
}

void main()
{
	const float waterLine = 0.003;
	const float snowLine = 0.2 + texture(u_texNoise, vec3(in_data.texCoord*10.0, in_data.elevation) * 0.05).r * 0.15;

	const vec2 texCoord = in_data.texCoord*120.0;

	vec3 normal = normalize(in_data.normal);
	
	//apply normal deviation in TBN space
	const vec3 matNormal = mix(vec3(0,0,1), (texture(u_texNormalMap, texCoord).rgb*2.0 - 1.0)*normalize(vec3(1,1,5)), step(waterLine, in_data.elevation));
	const mat3 tbn = cotangent_frame(normal, -getFragPos(vec3(gl_FragCoord.xy*2.0 - 1.0, 0)), texCoord);
	normal = normalize(tbn * matNormal);


	const vec3 normalWS = normalize( inverse(u_matNormal) * normal);

	
	const float grassFactor = 1.0 - smoothstep(0.07, 0.1, normalWS.y);
	FragColor.rgb = mix(texture(u_texGrass, texCoord).rgb, texture(u_texRock, texCoord*2.0).rgb, grassFactor);


	const float terrainHeight = texture(u_texHeight, in_data.texCoord).r;
	const float depth = in_data.elevation - terrainHeight;
	if (depth > 0.001)
	{
		FragColor.rgb = mix(FragColor.rgb, vec3(0.2, 0.3, 1.0), clamp(10.0*depth, 0.0, 1.0));
	}

	const float snowK = smoothstep(snowLine, snowLine + 0.05, in_data.elevation * smoothstep(0.001, 0.01, pow(normalWS.y, 2.0)));

	FragColor.rgb = mix(FragColor.rgb, vec3(0.97, 0.97, 1.0), snowK);
	FragColor.a = 1.0;

	FragNormal = vec4(normal, 1.0);

	RoughnessMetallic = vec4(mix(0.01 + smoothstep(0.0, waterLine, in_data.elevation) * 0.9, 0.5, snowK), 0.0, 1.0, 1.0);
}