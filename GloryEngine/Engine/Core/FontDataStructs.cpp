#include "FontDataStructs.h"

#include "FontData.h"
#include "RenderData.h"
#include "MeshData.h"
#include "VertexHelpers.h"

#include <queue>

namespace Glory::Utils
{
	void GenerateTextMesh(MeshData* pMesh, FontData* pFontData, const TextRenderData& renderData, float textWrap)
	{
		const std::string_view text = renderData.m_Text;
		const glm::vec4& color = renderData.m_Color;
		const float scale = renderData.m_Scale;
		const Alignment alignment = renderData.m_Alignment;
		textWrap = textWrap > 0.0f ? textWrap : renderData.m_TextWrap/scale;

		if (!renderData.m_Append)
		{
			pMesh->ClearVertices();
			pMesh->ClearIndices();
		}

		const size_t indexOffset = pMesh->VertexCount();

		/* Get words */
		std::vector<std::pair<size_t, size_t>> wordPositions;
		std::queue<size_t> newLineQueue;
		size_t currentStringPos = 0;

		while (currentStringPos < text.size())
		{
			const size_t nextSpace = text.find(' ', currentStringPos);
			const size_t nextNewLine = text.find('\n', currentStringPos);
			if (nextSpace == std::string::npos && nextNewLine == std::string::npos)
			{
				wordPositions.push_back({ currentStringPos, text.size() - currentStringPos });
				break;
			}
			const size_t first = std::min(nextSpace, nextNewLine);
			wordPositions.push_back({ currentStringPos, first - currentStringPos });
			const size_t wordCount = wordPositions.size();
			if (first == nextNewLine) newLineQueue.push(wordCount);
			currentStringPos = first + 1;
		}

		float writeX = 0.0f;
		float writeY = 0.0f;
		size_t letterCount = 0;
		/* For generating the mesh one line at a time */
		std::function<void(const std::string_view, float)> drawLine = [&](const std::string_view line, float lineWidth) {
			switch (alignment)
			{
			case Alignment::Left:
				writeX = 0.0f;
				break;
			case Alignment::Center:
				writeX = -lineWidth / 2.0f;
				break;
			case Alignment::Right:
				writeX = -lineWidth;
				break;
			default:
				break;
			}

			for (char c : line)
			{
				const size_t glyphIndex = pFontData->GetGlyphIndex(c == '\t' ? ' ' : c);
				const GlyphData* glyph = pFontData->GetGlyph(glyphIndex);

				if (!glyph) continue;

				const float xpos = renderData.m_Offsets.x + writeX + glyph->Bearing.x * scale;
				const float ypos = renderData.m_Offsets.y + writeY - (glyph->Size.y - glyph->Bearing.y) * scale;

				const float w = glyph->Size.x * scale;
				const float h = glyph->Size.y * scale;

				VertexPosColorTex vertices[4] = {
					{ { xpos, ypos + h, }, color, { glyph->Coords.x, glyph->Coords.y } },
					{ { xpos, ypos, }, color, { glyph->Coords.x, glyph->Coords.w } },
					{ { xpos + w, ypos, }, color, { glyph->Coords.z, glyph->Coords.w } },
					{ { xpos + w, ypos + h, }, color, { glyph->Coords.z, glyph->Coords.y }, }
				};

				pMesh->AddVertex(reinterpret_cast<float*>(&vertices[0]));
				pMesh->AddVertex(reinterpret_cast<float*>(&vertices[1]));
				pMesh->AddVertex(reinterpret_cast<float*>(&vertices[2]));
				pMesh->AddVertex(reinterpret_cast<float*>(&vertices[3]));
				pMesh->AddFace(indexOffset + letterCount*4 + 0, indexOffset + letterCount*4 + 1,
					indexOffset + letterCount*4 + 2, indexOffset + letterCount*4 + 3);
				++letterCount;
				writeX += (glyph->Advance >> 6) * scale;
			}

			writeY -= pFontData->FontHeight() * scale;
		};

		/* Calculate lines and generate the mesh */
		/* Process words into lines */
		size_t currentLineLength = 0;
		float currentLineWidth = 0.0f;
		size_t lineStart = 0;

		const size_t spaceGlyphIndex = pFontData->GetGlyphIndex(' ');
		const GlyphData* spaceGlyph = pFontData->GetGlyph(spaceGlyphIndex);

		size_t nextNewLineWord = std::string::npos;
		if (!newLineQueue.empty())
		{
			nextNewLineWord = newLineQueue.front();
			newLineQueue.pop();
		}

		const float space = spaceGlyph ? (spaceGlyph->Advance >> 6) * scale : 0.0f;
		for (size_t i = 0; i < wordPositions.size(); ++i)
		{
			const bool newLineBeforeWord = nextNewLineWord == i;
			if (newLineBeforeWord)
			{
				nextNewLineWord = std::string::npos;
				if (!newLineQueue.empty())
				{
					nextNewLineWord = newLineQueue.front();
					newLineQueue.pop();
				}
			}

			const std::pair<size_t, size_t>& wordPosition = wordPositions[i];
			const std::string_view word = text.substr(wordPosition.first, wordPosition.second);

			/* Calculate word width */
			float wordWidth = 0.0f;
			for (char c : word)
			{
				const size_t glyphIndex = pFontData->GetGlyphIndex(c == '\t' ? ' ' : c);
				const GlyphData* glyph = pFontData->GetGlyph(glyphIndex);

				if (!glyph) continue;
				const float advance = (glyph->Advance >> 6) * scale;
				wordWidth += advance;
			}

			if ((currentLineLength > 0 && textWrap > 0.0f && currentLineWidth + wordWidth >= textWrap) || newLineBeforeWord)
			{
				const std::string_view line = text.substr(lineStart, wordPosition.first - lineStart);
				drawLine(line, currentLineWidth);
				lineStart = wordPosition.first;
				currentLineWidth = 0.0f;
				currentLineLength = 0;
				--i;
				continue;
			}

			currentLineWidth += wordWidth + space;
			currentLineLength += word.length() + 1;
		}

		/* If there was 1 more line we could not generate yet we can generate it now */
		if (currentLineLength > 0)
		{
			const std::string_view line = text.substr(lineStart);
			drawLine(line, currentLineWidth);
		}
	}
}
