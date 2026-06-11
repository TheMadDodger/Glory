#ifndef DEVICE_OPENGL
#extension GL_EXT_nonuniform_qualifier : require
layout(set = 6, binding = 9) uniform sampler2D Textures2D[];
#else
layout(binding = 9) uniform sampler2D Textures2D[1024];
#endif

struct Texture2D
{
	uint ID;
};

vec4 SampleTexture2D(Texture2D textureId, vec2 coord)
{
#ifndef DEVICE_OPENGL
	return texture(Textures2D[nonuniformEXT(textureId.ID)], coord);
#else
	return texture(Textures2D[textureId.ID], coord);
#endif
}

const uint InvalidTextureID = ~0;