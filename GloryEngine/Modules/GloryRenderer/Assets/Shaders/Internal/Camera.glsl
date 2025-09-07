struct CameraData
{
	mat4 View;
	mat4 Projection;
	float zNear;
	float zFar;
	vec2 Resolution;
};

layout(set = 0, std140, binding = 1) readonly uniform CameraDatasUBO
{
	CameraData Cameras[100];
};

CameraData CurrentCamera()
{
	return Cameras[Constants.CameraIndex];
}
