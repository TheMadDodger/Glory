#type frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Internal/DepthHelpers.glsl"

layout(location = 0) in vec2 Coord;
layout(location = 0) out vec4 out_Color;

layout (binding = 0) uniform sampler2D Depth;

#ifdef PUSH_CONSTANTS
layout(push_constant) uniform RenderConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform RenderConstantsUBO
#endif
{
	uvec4 GridSize;
	uvec2 Resolution;
	float zNear;
    float zFar;
} Constants;

struct LightGridElement
{
    uint Offset;
    uint Count;
};

layout(set = 1, std430, binding = 1) buffer LightGridSSBO
{
    LightGridElement LightGrid[];
};

const vec4 GoodColor = vec4(0.0, 1.0, 0.0, 0.1);
const vec4 BadColor = vec4(1.0, 0.0, 0.0, 0.5);

void main()
{
	float depth = texture(Depth, Coord).r;
	float linearDepth = LinearDepth(depth, Constants.zNear, Constants.zFar);

	uint zTile = uint((log(abs(linearDepth)/Constants.zNear)*Constants.GridSize.z)/log(Constants.zFar/Constants.zNear));
    vec2 tileSize = Constants.Resolution/Constants.GridSize.xy;
    uvec3 tile = uvec3(gl_FragCoord.xy/tileSize, zTile);
    uint clusterID = tile.x + (tile.y*Constants.GridSize.x) + (tile.z*Constants.GridSize.x*Constants.GridSize.y);

	uint count = LightGrid[clusterID].Count;
	out_Color = mix(GoodColor, BadColor, count/50.0);
}
