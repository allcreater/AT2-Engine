#version 420 core

#define M_PI 3.1415926535897932384626433832795
#define M_INV_PI 0.31830988618379067153776752674503

vec4 getReflection(vec3 dir, float lod);




float lightAttenuation(in float distance, in float lightInnerR, in float lightOuterR)
{
	
    float d = max(distance, lightInnerR);
    return clamp(1.0 - pow(d / lightOuterR, 4.0), 0.0, 1.0) / (d * d + 1.0);
    

    float  attenuation = 1.0 - clamp(distance / lightOuterR, 0.0, 1.0);
    return attenuation * attenuation;
}

#define CookTorrance 1

#if (CookTorrance == 0)

//phong light model
vec3 computeLighting (
		in vec3 lightVec, in float lightRadius, in vec3 lightColor,
 		in vec3 normal, in vec3 viewDir,
		in float roughtness, in vec3 F0,
		in vec3 diffuseColor
 		)
{
	float lightDistance = length(lightVec);
	vec3 lightDir = lightVec / lightDistance; //normalize dir vector


	float Kd = max(dot (normal, lightDir), 0.0);
	float Ks = pow(max(dot (viewDir, reflect(-lightDir, normal)), 0.0), (1-roughtness)*200 + 1);

	float attenuation = (lightRadius > 0) ? lightAttenuation(lightDistance, 1.0, lightRadius) : 1.0;
	return lightColor * ((Kd*diffuseColor + Ks*F0) * attenuation);
}


#else

//CookTorrance light model
vec3 F_Schlick(in float cosT, in vec3 F0)
{
  //return vec3 (1.0);
  return F0 + (1.0 - F0) * pow( 1.0 - cosT, 5.0);
}

float D_Ggx(in float NdH, in float alpha)
{
	float alpha2 = alpha * alpha;
	float den = 1 + NdH*NdH * (alpha2 - 1);
	return 1/(M_PI * den * den);
} 

/*
float Gp_Ggx(float IdH, float IdN, float alpha)
{
    float chi = step( IdH / IdN );
    IdH = IdH * IdH;
    float tan2 = ( 1 - IdH ) / IdH;
    return (chi * 2) / ( 1 + sqrt( 1 + alpha * alpha * tan2 ) );
}
*/
float Gp_Ggx(in float IdN, in float alpha)
{
    IdN = IdN * IdN + 0.001;
    float tan2 = ( 1 - IdN ) / IdN;
    return 2 / ( 1 + sqrt( 1 + alpha * alpha * tan2 ) );
}

float G_SmithGgx(in float LdN, in float VdN, in float alpha)
{
	//G(l, v, h) = G'(l, h) * G'(v, h)
	//return 1;
	return Gp_Ggx(LdN, alpha) * Gp_Ggx(VdN, alpha);
}



float normal_distrib(
	float ndh,
	float Roughness)
{
// use GGX / Trowbridge-Reitz, same as Disney and Unreal 4
// cf http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf p3
	float alpha = Roughness * Roughness;
	float tmp = alpha / max(1e-8,(ndh*ndh*(alpha*alpha-1.0)+1.0));
	return tmp * tmp * M_INV_PI;
}

float G1(
	float ndw, // w is either Ln or Vn
	float k)
{
// One generic factor of the geometry function divided by ndw
// NB : We should have k > 0
	return 1.0 / ( ndw*(1.0-k) + k );
}

float visibility(
	float ndl,
	float ndv,
	float Roughness)
{
// Schlick with Smith-like choice of k
// cf http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf p3
// visibility is a Cook-Torrance geometry function divided by (n.l)*(n.v)
	float k = Roughness * Roughness * 0.5;
	return G1(ndl,k)*G1(ndv,k);
}


vec3 computeLighting (
		in vec3 lightVec, in float lightRadius, in vec3 lightColor,
 		in vec3 normal, in vec3 viewDir,
		in float roughtness, in vec3 F0,
		in vec3 diffuseColor
 		)
{
	float lightDistance = length(lightVec);
	vec3 lightDir = lightVec / lightDistance; //normalize dir vector


	vec3 halfVec = normalize(viewDir + lightDir);

	float NdV = clamp(dot(normal, viewDir), 0.0, 1.0);
	float NdL = clamp(dot(normal, lightDir), 0.0, 1.0);
	float VdH = clamp(dot(viewDir, halfVec), 0.0, 1.0);
	float NdH = clamp(dot(normal, halfVec), 0.0, 1.0);

	

	//float Kd = NdL;
	//float Ks = pow(NdH, 100.0);


	float alpha = pow(roughtness, 2.0);

	//F = (D(h) * G(l, v, h) * F(v, h))/(4 * NdL * NdV)
	vec3 F = D_Ggx(NdH, alpha) * G_SmithGgx(NdL, NdV, alpha) * F_Schlick(NdV, F0);
	F /= (4 * NdL * NdV + 0.01);


	F = (F_Schlick(VdH, F0) * ( normal_distrib(NdH, roughtness ) * visibility(NdL,NdV,roughtness) / 4.0 ) + diffuseColor*(vec3(1.0)-F0)) * NdL;

	float attenuation = (lightRadius > 0) ? lightAttenuation(lightDistance, 1.0, lightRadius) : 1.0;
	return lightColor * (F * attenuation);
}

// Hammersley function (return random low-discrepency points)
vec2 Hammersley(uint i, uint N)
{
	return vec2(
		float(i) / float(N),
		float(bitfieldReverse(i)) * 2.3283064365386963e-10
	);
}
 
// Random rotation based on the current fragment's coordinates
float randAngle()
{
	uint x = uint(gl_FragCoord.x);
	uint y = uint(gl_FragCoord.y);
	return float(30u * x ^ y + 10u * x * y);
}

// Example function, skewing the sample point and rotating
// Note: E is two values returned from Hammersley function above,
//       from within the same loop.
vec2 DGgxSkew(vec2 E, float roughness)
{
  float a = roughness * roughness;
  E.x = atan(sqrt((a * E.x) / (1.0 - E.x)));
  E.y = M_PI*2 * E.y + randAngle();
  return E;
}
 
// Example function, turn a skewed sample into a 3D vector
// This results in a vector that is looking somewhere in
// the +Z Hemisphere.
vec3 MakeSample(vec2 E)
{
  float SineTheta = sin(E.x);
 
  float x = cos(E.y) * SineTheta;
  float y = sin(E.y) * SineTheta;
  float z = cos(E.x);
 
  return vec3(x, y, z);
}

// Computes the exact mip-map to reference for the specular contribution.
// Accessing the proper mip-map allows us to approximate the integral for this
// angle of incidence on the current object.
float compute_lod(uint NumSamples, float NoH, float roughness)
{
  const vec2 Dimensions = vec2(2048, 1024);
  float dist = D_Ggx(NoH, roughness); // Defined elsewhere as subroutine
  return 0.5 * (log2(float(Dimensions.x * Dimensions.y) / NumSamples) - log2(dist));
}
 

vec3 computeIBL (
	in int numSamples,
	in vec3 normal, in vec3 viewDir,
	in float roughness, in vec3 F0
)
{

	// Precalculate rotation for +Z Hemisphere to microfacet normal.
	vec3 UpVector = abs(normal.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
	vec3 TangentX = normalize(cross( UpVector, normal ));
	vec3 TangentY = cross(normal, TangentX);
	mat3 tbn = mat3(TangentX, TangentY, normal);

	float NdV =  max( 1e-8, abs(dot( normal, viewDir )) );

	float alpha = roughness * roughness;

	vec3 lighting = vec3(0);
	for (int i = 0; i < numSamples; i++)
	{
		vec2 Xi = Hammersley(i, numSamples);

		vec3 halfVec = normalize(tbn * MakeSample(DGgxSkew(Xi, roughness)));
		vec3 lightVec = -reflect(-viewDir, halfVec);

		float NdL = max( 1e-8, abs(dot(normal, lightVec)) );
		float VdH = max( 1e-8, abs(dot(viewDir, halfVec)) );
		float NdH = max( 1e-8, abs(dot(normal, halfVec)) );

		float lod = compute_lod(numSamples, NdH, roughness);
		vec3 color = getReflection(lightVec, lod).rgb;
		
		vec3 F = color * F_Schlick(VdH, F0) * ( visibility(NdL,NdV,roughness) * VdH * NdL) / NdH;
		lighting += F;
	}

	return lighting/numSamples;
}

#endif