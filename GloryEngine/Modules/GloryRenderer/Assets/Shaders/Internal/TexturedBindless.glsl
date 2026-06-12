#ifndef DEVICE_OPENGL
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_nonuniform_qualifier : require
layout(set = 6, binding = 9) uniform sampler2D Textures2D[];
#else
layout(binding = 9) uniform sampler2D Textures2D[1024];
#endif

const vec4 MissingTextureColor = vec4(1.0, 0.753, 0.796, 1.0);
const uint InvalidTextureID = ~0;

struct Texture2D
{
	uint ID;
};

vec4 SampleTexture2D(Texture2D textureId, vec2 coord, vec4 defaultValue)
{
	if (textureId.ID == InvalidTextureID)
		return defaultValue;
#ifndef DEVICE_OPENGL
	return texture(Textures2D[nonuniformEXT(textureId.ID)], coord);
#else
	return texture(Textures2D[textureId.ID], coord);
#endif
}
