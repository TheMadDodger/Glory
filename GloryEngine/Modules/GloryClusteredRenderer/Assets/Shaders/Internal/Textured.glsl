#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout(std430, binding = 3) readonly buffer HasTextureSSBO
{
    uint64_t HasTexture;
};

bool TextureEnabled(int index)
{
	uint64_t bit = 1 << index;
	return (HasTexture & bit) > 0;
}