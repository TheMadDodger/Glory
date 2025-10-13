#type frag

#version 450
#extension GL_ARB_separate_shader_objects : enable

#define FEATURE_TRANSPARENT_TEXTURED

#include "internal/RenderConstants.glsl"

#ifdef WITH_TRANSPARENT_TEXTURED
#include "internal/Textured.glsl"
layout(binding = 0) uniform sampler2D texSampler;
layout(location = 0) in vec2 fragTexCoord;
#endif

void main()
{
#ifdef WITH_TRANSPARENT_TEXTURED
	if (TextureEnabled(0) && texture(texSampler, fragTexCoord).a == 0.0) discard;
#endif
}
