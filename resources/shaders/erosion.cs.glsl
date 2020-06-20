#version 430
layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba16f/*, binding = 0*/) uniform image2D u_result;


float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

float computeHeight(in vec2 pos)
{
    float height = 0.0;
    float amplitude = 0.5;
    float frequency = 10.0;
    for (int i = 0; i < 10; ++i)
    {
        height += noise(pos*frequency + vec2(100 * i)) * amplitude;
        amplitude *= 0.45;
        frequency *= 2.0;
    }

    return height * 2.0 - 0.5;
}

vec3 getNormal(in ivec2 pos, in float flatness)
{
    pos = clamp(pos, ivec2(1, 1), imageSize(u_result)-2);

    const ivec2 dx = ivec2(1, 0);
    const ivec2 dy = ivec2(0, 1);

    return normalize(vec3(
        imageLoad(u_result, pos - dx).r - imageLoad(u_result, pos + dx).r,
        flatness,
        imageLoad(u_result, pos - dy).r - imageLoad(u_result, pos + dy).r
    ));
}

void trace (in vec2 pos)
{
    const int maxIterations = 80;
    const float radius = 0.8;
    const float depositionRate = 0.03;
    const float erosionRate = 0.04;
    const float iterationScale = 0.04;
    const float friction = 0.77;
    const float speed = 0.15;

    const vec2 dir = (vec2(rand(pos), rand(pos + vec2(100))) * 2.0 - 1.0) * radius;

    float sediment = 0.0;
    vec2 prevPos = pos;
    vec2 velocity = vec2(0.0);
    for (int i = 0; i < maxIterations; ++i)
    {
        const vec3 surfaceNormal = getNormal(ivec2(pos + dir), 0.0005);
        if (surfaceNormal.y == 1.0)
            break;

        const float deposit = sediment * depositionRate * surfaceNormal.y;
        const float erosion = erosionRate * (1 - surfaceNormal.y) * min(1, i * iterationScale);


        //yep, not atomic =(
        vec4 texel = imageLoad(u_result, ivec2(prevPos));
        texel.x += (deposit - erosion);
        imageStore(u_result, ivec2(prevPos), texel);

        sediment += erosion - deposit;
        velocity = velocity * friction + surfaceNormal.xz * speed;
        prevPos = pos;
        pos += velocity;
    }
}

void smoothTerrain()
{

}


void main() 
{
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 targetSize = imageSize(u_result); 

    vec4 pixel = vec4(computeHeight(vec2(pixelCoords)/vec2(targetSize)), 0.0, 1.0, 1.0);

    imageStore(u_result, pixelCoords, pixel);

    barrier();

    for (int )
}