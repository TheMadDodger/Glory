#version 450
#extension GL_ARB_separate_shader_objects : enable

uniform float _u_fragScalar = 1.0;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragPosition;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;

void main()
{
	outColor = texture2D(texSampler, fragTexCoord) * _u_fragScalar;
	outPosition = fragPosition;
	outNormal = vec4(fragNormal, 1.0);
}
