#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPos;
layout(location = 2) out vec4 outNormal;

void main()
{
	//outColor = vec4(fragColor * texture(texSampler, fragTexCoord * 2.0).rgb, 1.0);
	outPos = fragPos;
	outNormal = vec4(fragNormal * 0.5 + 1.0, 1.0);
	outColor = texture(texSampler, fragTexCoord);
}
