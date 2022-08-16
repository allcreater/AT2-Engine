#version 410 core

uniform CameraBlock
{
	mat4 u_matView, u_matInverseView, u_matProjection, u_matInverseProjection, u_matViewProjection;
    double u_time;
};
uniform mat4 u_matModel;
uniform mat3 u_matNormal;


uniform sampler2D u_texAlbedo;
uniform sampler2D u_texNormalMap;
uniform sampler2D u_texAoRoughnessMetallic;

in fsInput {
	vec3 texCoord;
	vec3 normal;
	vec3 position; //in view-space
} in_data;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragNormal;
layout (location = 2) out vec4 RoughnessMetallic; 

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

//SRGB -> RGB
vec3 to_linear(in vec3 color)
{
    const float gamma = 2.2f;
    return pow(color, vec3(gamma));
}

void main()
{
	//vec3 normalFromMap = (texture(u_texNormalMap, in_data.texCoord).rbg * 2.0 - 1.0)*vec3(1.0, 1.0, -1.0);

	vec2 texCoord = in_data.texCoord.st;

    vec4 albedo = texture(u_texAlbedo, texCoord);
    albedo.rgb = to_linear(albedo.rgb);

    vec4 aorm_sample = texture(u_texAoRoughnessMetallic, texCoord);

    float ao = aorm_sample.r;
    float roughness = aorm_sample.g + 0.01;
    float metallic = aorm_sample.b;

    if (albedo.a < 0.5)
        discard;
        
    vec4 final_color = vec4(albedo.rgb, 1.0);
    //final_color.rgb *= ao;

	FragColor.rgba = final_color;


	vec3 normal = normalize(in_data.normal);

	vec3 matNormal = (texture(u_texNormalMap, texCoord).rgb*2.0 - 1.0);
	mat3 tbn = cotangent_frame(normal, -getFragPos(vec3(gl_FragCoord.xy*2.0 - 1.0, 0)), texCoord);

	FragNormal = vec4(tbn * matNormal, 1.0);


    RoughnessMetallic = vec4(roughness, metallic, 1.0, 1.0);
}