struct CameraData
{
	mat4 View;
	mat4 Projection;
	float zNear;
	float zFar;
	vec2 Resolution;
};

layout(set = 0, std430, binding = 1) readonly buffer CameraDatasUBO
{
	CameraData Cameras[100];
};

CameraData CurrentCamera()
{
	return Cameras[Constants.CameraIndex];
}
