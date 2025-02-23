#type frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 Coord;
layout(location = 0) out vec4 out_Color;
layout (binding = 3) uniform sampler2D Normal;
layout (binding = 5) uniform sampler2D Depth;
layout (binding = 6) uniform sampler2D Noise;

#include "Internal/DepthHelpers.glsl"

layout(std430, binding = 3) buffer sampleDome
{
    vec3 SamplePoints[];
};

layout(std430, binding = 6) buffer ssaoSettings
{
    int AOEnabled;
    int Dirty;
    float SampleRadius;
    float SampleBias;
    int KernelSize;
    int BlurType;
    int BlurSize;
    float Separation;
    int BinsSize;
    float Magnitude;
    float Contrast;
};

void main()
{
    vec2 noiseScale = vec2(ScreenDimensions.x/4.0, ScreenDimensions.y/4.0);

	vec3 normal = texture(Normal, Coord).xyz * 2.0 - 1.0;
    normal = mat3(inverse(ViewInverse)) * normalize(normal);
	vec3 randomVec = texture(Noise, Coord*noiseScale).xyz;
	float depth = texture(Depth, Coord).r;
	vec4 fragPosition = ViewPosFromDepth(depth);

    vec3 tangent   = normalize(randomVec - normal*dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = float(KernelSize);
    for(int i = 0; i < KernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * SamplePoints[i]; // from tangent to view-space
        samplePos = fragPosition.xyz + samplePos*SampleRadius; 

        vec4 offset = vec4(samplePos, 1.0);
        offset      = inverse(ProjectionInverse)*offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        float sampleDepth = texture(Depth, offset.xy).r;
        vec3 offsetPosition = ViewPosFromDepth(sampleDepth).xyz;
        float intensity = smoothstep(0.0, 1.0, SampleRadius/abs(fragPosition.z - offsetPosition.z));
        float occluded = samplePos.z + SampleBias <= offsetPosition.z ? 1.0 : 0.0;
        occluded *= intensity;
        occlusion -= occluded;
    }

    occlusion = occlusion/KernelSize;
	out_Color = vec4(vec3(occlusion), fragPosition.a);
}
