#type frag
#version 450

layout(location = 0) in vec2 Coord;
layout(location = 0) out vec4 out_Color;
layout (binding = 0) uniform sampler2D Color;

void main()
{
	vec4 color = texture(Color, Coord);
	out_Color = color;
}
