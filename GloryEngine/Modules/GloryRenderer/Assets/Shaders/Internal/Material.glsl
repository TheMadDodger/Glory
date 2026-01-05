layout(set = 4, binding = 8, std430) readonly buffer MaterialSSBO
{
	Material Materials[];
};

Material GetMaterial()
{
	return Materials[Constants.MaterialIndex];
}