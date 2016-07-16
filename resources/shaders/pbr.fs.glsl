#version 420 core

float lightAttenuation(in float distance, in float lightInnerR, in float invLightOuterR)
{
	
    float d = max(distance, lightInnerR);
    return clamp(1.0 - pow(d * invLightOuterR, 4.0), 0.0, 1.0) / (d * d + 1.0);
    

    float  attenuation = 1.0 - clamp(distance * invLightOuterR, 0.0, 1.0);
    return attenuation * attenuation;
}

vec3 computeLighting (
		in vec3 lightVec, in float lightRadius, in vec3 lightColor,
 		in vec3 normal, in vec3 viewDir
 		)
{
	float lightDistance = length(lightVec);
	vec3 lightDir = lightVec / lightDistance; //normalize dir vector


	float Kd = max(dot (normal, -lightDir), 0.0);
	float Ks = pow(max(dot (normal, normalize(viewDir + lightDir)), 0.0), 70.0);
	return lightColor * ((Kd + Ks) * lightAttenuation(lightDistance, 10.0, 1.0/lightRadius));
}