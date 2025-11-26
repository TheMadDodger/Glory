#type frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

#define FEATURE_RED_CHANNEL_ONLY
#define FEATURE_USAMPLER

layout(location = 0) in vec2 Coord;
layout(location = 0) out vec4 out_Color;

#ifdef WITH_USAMPLER
layout (binding = 0) uniform usampler2D Color;
#else
layout (binding = 0) uniform sampler2D Color;
#endif

void main()
{
#ifdef WITH_RED_CHANNEL_ONLY
	out_Color = vec4(vec3(texture(Color, Coord).r), 1.0);
#else

#ifdef WITH_USAMPLER
	uvec4 ucolor = texture(Color, Coord);
	vec4 color = ucolor/4294967295.0;
	color *= color.a;
	color.a = 1.0;
	out_Color = color;
#else
	out_Color = texture(Color, Coord);
#endif

#endif
}
