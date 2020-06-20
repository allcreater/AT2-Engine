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


void main() 
{
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 targetSize = imageSize(u_result); 

    vec4 pixel = vec4(computeHeight(vec2(pixelCoords)/vec2(targetSize)), 0.0, 1.0, 1.0);

    imageStore(u_result, pixelCoords, pixel);
}