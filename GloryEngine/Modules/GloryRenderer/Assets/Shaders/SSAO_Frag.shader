#type frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 Coord;
layout(location = 0) out float out_Color;
layout (set = 2, binding = 0) uniform sampler2D Normal;
layout (set = 2, binding = 1) uniform sampler2D Depth;
layout (set = 3, binding = 2) uniform sampler2D Noise;

#ifdef PUSH_CONSTANTS
layout(push_constant) uniform RenderConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform RenderConstantsUBO
#endif
{
	uint CameraIndex;
    int KernelSize;
    float SampleRadius;
    float SampleBias;
} Constants;

#include "Internal/Camera.glsl"
#include "Internal/DepthHelpers.glsl"

layout(set = 1, std140, binding = 2) readonly uniform SampleDomeUBO
{
    vec3 SamplePoints[64];
};

void main()
{
    float depth = texture(Depth, Coord).r;
    if(depth == 1.0)
    {
        out_Color = 1.0;
        return;
    }

    CameraData camera = CurrentCamera();
    vec2 noiseScale = vec2(camera.Resolution.x/4.0, camera.Resolution.y/4.0);
	vec3 normal = texture(Normal, Coord).xyz*2.0 - 1.0;
    normal = mat3(camera.View)*normalize(normal);
	vec3 randomVec = texture(Noise, Coord*noiseScale).xyz;

	vec4 fragPosition = ViewPosFromDepth(depth, camera.ProjectionInverse, Coord);

    vec3 tangent   = normalize(randomVec - normal*dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = float(Constants.KernelSize);
    for(int i = 0; i < Constants.KernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN*SamplePoints[i]; // from tangent to view-space
        samplePos = fragPosition.xyz + samplePos*Constants.SampleRadius; 

        vec4 offset = vec4(samplePos, 1.0);
        offset      = camera.Projection*offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        float sampleDepth = texture(Depth, offset.xy).r;
        vec3 offsetPosition = ViewPosFromDepth(sampleDepth, camera.ProjectionInverse, Coord).xyz;
        float intensity = smoothstep(0.0, 1.0, Constants.SampleRadius/abs(fragPosition.z - offsetPosition.z));
        float occluded = samplePos.z + Constants.SampleBias <= offsetPosition.z ? 1.0 : 0.0;
        occluded *= intensity;
        occlusion -= occluded;
    }

    occlusion = occlusion/Constants.KernelSize;
	//out_Color = vec4(vec3(occlusion), fragPosition.a);
    out_Color = occlusion;
}
