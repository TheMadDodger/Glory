layout(set = 1, binding = 4, std430) readonly buffer MaterialSSBO
{
	Material Materials[];
};

Material GetMaterial()
{
	return Materials[Constants.MaterialIndex];
}