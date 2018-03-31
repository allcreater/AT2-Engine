#version 420 core

uniform vec2 u_BoundsX;
uniform uint u_NumberOfPoints;
uniform mat4 u_matProjection;

//layout(location = 0) in vec4 a_Color;
layout(location = 0) in float a_Position;

void main()
{
	float x = mix(u_BoundsX.x, u_BoundsX.y, float(gl_VertexID)/float(u_NumberOfPoints));
	vec4 pos = vec4(x, a_Position, 0.0, 1.0);
	gl_Position = u_matProjection * pos;


}
