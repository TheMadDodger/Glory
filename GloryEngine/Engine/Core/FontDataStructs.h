#pragma once
#include "UUID.h"

#include <vector>
#include <glm/ext/vector_int2.hpp>
#include <glm/vec4.hpp>

namespace Glory
{
	struct GlyphData
	{
		uint64_t Code;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		uint32_t Advance;
		glm::vec4 Coords;
	};

	struct TextRenderData;
	class MeshData;
	class FontData;

	namespace Utils
	{
		void GenerateTextMesh(MeshData* pMesh, FontData* pFontData, const TextRenderData& renderData, float textWrap=0.0f);
	}
}
