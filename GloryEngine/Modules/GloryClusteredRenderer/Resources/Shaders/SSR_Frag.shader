#version 450
#extension GL_ARB_separate_shader_objects : enable

in vec2 Coord;
layout(location = 0) out vec4 out_Color;
layout (binding = 1) uniform sampler2D Color;
layout (binding = 2) uniform sampler2D Normal;
layout (binding = 3) uniform sampler2D Debug;
layout (binding = 4) uniform sampler2D Depth;

//uniform vec3 eyeDirection;

uniform float zNear;
uniform float zFar;

float LinearDepth(float depthSample);
vec3 WorldPosFromDepth(float depth);

layout(std430, binding = 2) buffer screenToView
{
    mat4 ProjectionInverse;
    mat4 ViewInverse;
    uvec4 TileSizes;
    uvec2 ScreenDimensions;
    float Scale;
    float Bias;
};

void main()
{
	vec3 color = texture2D(Color, Coord).xyz;
	vec3 normal = texture2D(Normal, Coord).xyz * 2.0 - 1.0;
	float depth = texture2D(Depth, Coord).r;
	vec3 fragPosition = WorldPosFromDepth(depth);

	vec3 lightReflected = reflect(lightVec, normal);

	vec3 diffuseColor = color;

	out_Color = vec4(diffuseColor, 1.0);
}

float LinearDepth(float depthSample)
{
    float depthRange = 2.0 * depthSample - 1.0;
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}

vec3 WorldPosFromDepth(float depth)
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(Coord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ProjectionInverse * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = ViewInverse * viewSpacePosition;

    return worldSpacePosition.xyz;
}
