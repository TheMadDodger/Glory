#type frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Internal/RenderConstants.glsl"

struct Material
{
	vec4 Color;
	float Shininess;
};

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out uvec4 outID;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outNormal;
layout(location = 5) out vec4 outData;

void main()
{
	float pixel = texture(texSampler, fragTexCoord).r;
	if (pixel < 1.0) discard;
	outColor = pixel*inColor;
	outNormal = vec4((normalize(inNormal) + 1.0)*0.5, 1.0);
	outID = Constants.ObjectID;
	outData.a = 1.0;
}
