#version 420 core

uniform vec2 u_BoundsX;
uniform uint u_NumberOfPoints;

//layout(location = 0) in vec4 a_Color;
layout(location = 1) in float a_Position;

out vec4 v_Color;

void main()
{
	gl_Position = vec4(mix(u_BoundsX.x, u_BoundsX.y, float(gl_VertexID)/u_NumberOfPoints), a_Position, 0.0, 1.0);
	//v_Color = a_Color;
}
