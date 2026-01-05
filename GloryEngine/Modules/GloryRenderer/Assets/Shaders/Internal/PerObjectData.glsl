layout(set = 1, std430, binding = 2) readonly buffer WorldTransformsSSBO
{
	mat4 Worlds[];
};

mat4 WorldTransform()
{
	return Worlds[Constants.ObjectDataIndex];
}
