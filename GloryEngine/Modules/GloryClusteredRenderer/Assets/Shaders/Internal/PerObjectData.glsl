struct CameraData
{
	mat4 View;
	mat4 Projection;
};

layout(std140, binding = 2) readonly uniform CameraDatas
{
	CameraData Cameras[100];
};

layout(std430, binding = 3) readonly buffer WorldTransforms
{
	mat4 Worlds[];
};