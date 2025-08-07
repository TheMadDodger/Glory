#ifdef PUSH_CONSTANTS
layout(push_constant) uniform RenderConstants
#else
layout(std140, binding = 1) readonly uniform RenderConstants
#endif
{
	uvec4 ObjectID;
	uint ObjectDataIndex;
	uint CameraIndex;
} Constants;