layout(set = 1, binding = 3, std430) readonly buffer MaterialSSBO
{
	Material Materials[];
};

Material GetMaterial()
{
	return Materials[Constants.MaterialIndex];
}