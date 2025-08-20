struct CameraData
{
	mat4 View;
	mat4 Projection;
};

layout(std140, binding = 2) readonly uniform CameraDatasUBO
{
	CameraData Cameras[100];
};

layout(std430, binding = 3) readonly buffer WorldTransformsSSBO
{
	mat4 Worlds[];
};

mat4 WorldTransform()
{
	return Worlds[Constants.ObjectDataIndex];
}

CameraData CurrentCamera()
{
	return Cameras[Constants.CameraIndex];
}