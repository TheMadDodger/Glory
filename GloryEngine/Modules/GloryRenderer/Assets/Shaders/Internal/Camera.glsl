struct CameraData
{
	mat4 View;
	mat4 Projection;
	mat4 ViewInverse;
	mat4 ProjectionInverse;
	float zNear;
	float zFar;
	vec2 Resolution;
};

layout(set = 0, std430, binding = 1) readonly buffer CameraDatasSSBO
{
	CameraData Cameras[100];
};

CameraData CurrentCamera()
{
	return Cameras[Constants.CameraIndex];
}
