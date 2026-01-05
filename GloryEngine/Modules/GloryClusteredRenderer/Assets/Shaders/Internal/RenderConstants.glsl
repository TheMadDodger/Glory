#ifdef PUSH_CONSTANTS
layout(push_constant) uniform RenderConstantsUBO
#else
layout(set = 0, std140, binding = 1) readonly uniform RenderConstantsUBO
#endif
{
	uvec4 ObjectID;
	uint ObjectDataIndex;
	uint CameraIndex;
	uint MaterialIndex;
} Constants;