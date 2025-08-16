#pragma once
#include "UUID.h"

#include <vector>
#include <glm/ext/vector_int2.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <Reflection.h>

REFLECTABLE_ENUM_NS(Glory, Alignment, Left, Center, Right)

namespace Glory
{
	struct TextData
	{
		/* Settings */
		bool m_TextDirty;
		float m_Scale;
		Alignment m_Alignment;
		float m_TextWrap;

		/* Text */
		std::string m_Text;
		glm::vec4 m_Color;

		glm::vec2 m_Offsets{};
		bool m_Append{ false };
	};

	struct GlyphData
	{
		uint64_t Code;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		uint32_t Advance;
		glm::vec4 Coords;
	};

	class MeshData;
	class FontData;

	namespace Utils
	{
		void GenerateTextMesh(MeshData* pMesh, FontData* pFontData, const TextData& renderData, float textWrap=0.0f);
	}
}
