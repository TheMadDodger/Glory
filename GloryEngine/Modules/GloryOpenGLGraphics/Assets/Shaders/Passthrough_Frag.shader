#type frag
#version 450

layout(location = 0) in vec2 Coord;
layout(location = 0) out float out_Color;
layout (binding = 0) uniform sampler2D Color;

void main()
{
	float color = texture(Color, Coord).r;
	out_Color = color;
}
