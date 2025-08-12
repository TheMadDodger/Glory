#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout(std430, binding = 6) readonly buffer HasTextureSSBO
{
    uint HasTexture[];
};

bool TextureEnabled(int index)
{
	uint bit = 1 << index;
	return (HasTexture[Constants.MaterialIndex] & bit) > 0;
}