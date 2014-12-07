#version 400 core

precision mediump float;

uniform CameraBlock
{
	mat4 u_matMW, u_matInverseMW, u_matProj, u_matInverseProj;
};

uniform float u_phase;
uniform sampler3D u_texNoise;

uniform vec3 u_sunDirection = normalize(vec3(0.0, 1.0, 0.2));

uniform vec3 u_skyColor = vec3(0.4, 0.4, 1.0);
uniform vec3 u_cloudColor = vec3(1.0, 1.0, 1.0);


in vec4 v_color;
in vec2 v_texCoord;

out vec4 FragColor;


vec3 getEyeDir()
{
    vec4 device_normal = vec4(v_texCoord*2.0-1.0, 0.0, 1.0);
    vec3 eye_normal = normalize((u_matInverseProj * device_normal).xyz);
    vec3 world_normal = normalize(mat3(u_matInverseMW) * eye_normal);
    
    return world_normal;
}

float getDensity(vec3 pos)
{
    float summ = 0.0;
    vec3 bias = vec3(0);
    float freq = 1.0;
    for(int i = 0; i < 6; i++)
    {
        summ += texture(u_texNoise, pos*freq*0.2 + bias).b/(freq*2.0);
        freq *= 2.0;
        bias += vec3(0.17);
    }

    float cloudDensity = smoothstep(0.5, 1.0, summ)*0.4;
    return smoothstep(-0.01,0.01,pos.y*10.0)*smoothstep(-0.01,0.01,(0.1-pos.y)*2.0)*cloudDensity;
}

float traceAttenuation(in vec3 pos, in vec3 dpos)
{
	float accum = 0.0;
	for(int i = 0; i < 60; ++i)
	{
		accum += getDensity(pos);
		pos += dpos;
		
		if (accum > 1.0)
			break;
	}

	return accum*0.3;
}

void main()
{
	vec3 dir = getEyeDir();

	//FragColor = vec4(dir*0.5+0.5, 1.0);
	
	
	vec3 integralColor = vec3(0.0);
	float integralDensity = 0.0;
	vec3 rayPos = u_matInverseMW[3].xyz * 0.001;
	for(int i = 0; i < 60; ++i)
	{
		float density = getDensity(rayPos);
		
		if (density > 0.01)
		{
			vec3 color = mix(u_cloudColor, vec3(0.0), traceAttenuation(rayPos, u_sunDirection*0.001));
			//integralColor = mix(mix(integralColor,color,density), integralColor, integralDensity);
			integralColor += (color * density*2.0 * (1.0 - integralDensity));
			integralDensity += density;
		}
		
		rayPos += dir*0.001;

		if (integralDensity > 1.0)
			break;
	}

	integralColor.r = min(integralColor.r, 1.0); integralColor.g = min(integralColor.g, 1.0); integralColor.b = min(integralColor.b, 1.0);
	//FragColor = vec4(mix(u_skyColor, integralColor, integralDensity), 1.0);
	FragColor = vec4(integralColor, integralDensity);

	//FragColor = vec4(vec3(Noise(dir*5.0)), 1.0);
}
