#ifdef PUSH_CONSTANTS
layout(push_constant) uniform RenderConstantsUBO
#else
layout(set = 0, std140, binding = 0) readonly uniform RenderConstantsUBO
#endif
{
	uvec4 GridSize;
	uvec4 ObjectID;
	uint ObjectDataIndex;
	uint CameraIndex;
	uint MaterialIndex;
	uint LightCount;
} Constants;