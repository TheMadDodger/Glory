#ifndef DEVICE_OPENGL
#extension GL_EXT_nonuniform_qualifier : require
layout(set = 6, binding = 9) uniform sampler2D Textures2D[];
#else
layout(set = 6, binding = 9) uniform sampler2D Textures2D[1];
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
	return vec4(1.0);
#endif
}

const uint InvalidTextureID = ~0;