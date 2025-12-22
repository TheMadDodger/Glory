layout(set = 0, std430, binding = 1) readonly buffer WorldTransformsSSBO
{
	mat4 Worlds[];
};

layout(set = 0, std430, binding = 2) readonly buffer ColorsSSBO
{
	vec4 Colors[];
};

mat4 WorldTransform()
{
	return Worlds[Constants.ObjectIndex];
}

vec4 Color()
{
	return Colors[Constants.ColorIndex];
}