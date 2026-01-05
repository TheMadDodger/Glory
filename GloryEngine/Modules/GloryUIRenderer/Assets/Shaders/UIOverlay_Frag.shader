#type frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D Color;
layout(set = 1, binding = 1) uniform sampler2D UIColor;
layout(location = 0) in vec2 Coord;
layout(location = 0) out vec4 out_Color;

void main()
{
	vec4 color = texture(Color, Coord);
	vec4 uiColor = texture(UIColor, Coord);
	out_Color = vec4(color.xyz * (1.0 - uiColor.a) + uiColor.xyz * uiColor.a, 1.0);
}
