#include "UIDocument.h"
#include "UIDocumentData.h"
#include "UIComponents.h"

#include <FontData.h>
#include <MeshData.h>
#include <RenderData.h>
#include <VertexHelpers.h>

namespace Glory
{
	void UIDocument::CopyEntity(Utils::ECS::EntityRegistry& registry, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent)
	{
		const Utils::ECS::EntityID newEntity = registry.CopyEntityToOtherRegistry(entity, parent, &m_Registry);
		Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
		{
			const Utils::ECS::EntityID child = pEntityView->Child(i);
			CopyEntity(registry, child, newEntity);
		}
	}

	UIDocument::UIDocument(UIDocumentData* pDocument):
		m_OriginalDocumentID(pDocument->GetUUID()), m_pUITexture(nullptr),
		m_Name(pDocument->m_Name), m_Ids(pDocument->m_Ids),
		m_UUIds(pDocument->m_UUIds), m_Names(pDocument->m_Names)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->GetRegistry();
		for (size_t i = 0; i < registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = registry.Child(0, i);
			CopyEntity(registry, child, 0);
		}
	}

	RenderTexture* UIDocument::GetUITexture()
	{
		return m_pUITexture;
	}

	void UIDocument::Update()
	{
		m_Registry.SetUserData(this);
		m_Registry.InvokeAll(Utils::ECS::InvocationType::Update, NULL);
	}

	void DrawEntity(Utils::ECS::EntityID entity, Utils::ECS::EntityRegistry& registry)
	{
		registry.InvokeAll(Utils::ECS::InvocationType::Draw, { entity });
		for (size_t i = 0; i < registry.ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = registry.Child(entity, i);
			DrawEntity(child, registry);
		}
	}

	void UIDocument::Draw()
	{
		m_Registry.SetUserData(this);
		for (size_t i = 0; i < m_Registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(0, i);
			DrawEntity(child, m_Registry);
		}
	}

	UUID UIDocument::OriginalDocumentID() const
	{
		return m_OriginalDocumentID;
	}

	UIRendererModule* UIDocument::Renderer()
	{
		return m_pRenderer;
	}

	glm::mat4& UIDocument::Projection()
	{
		return m_Projection;
	}

	const glm::mat4& UIDocument::Projection() const
	{
		return m_Projection;
	}

	void GenerateTextMesh(MeshData* pMesh, FontData* pFontData, const TextRenderData& renderData)
	{
		const std::string_view text = renderData.m_Text;
		const glm::vec4& color = renderData.m_Color;
		const float scale = renderData.m_Scale;
		const Alignment alignment = renderData.m_Alignment;
		const float textWrap = renderData.m_TextWrap / scale;

		pMesh->ClearVertices();
		pMesh->ClearIndices();

		/* Get words */
		std::vector<std::pair<size_t, size_t>> wordPositions;
		size_t currentStringPos = 0;

		while (currentStringPos < text.size())
		{
			const size_t nextSpace = text.find(' ', currentStringPos);
			if (nextSpace == std::string::npos)
			{
				wordPositions.push_back({ currentStringPos, text.size() - currentStringPos });
				break;
			}
			wordPositions.push_back({ currentStringPos, nextSpace - currentStringPos });
			currentStringPos = nextSpace + 1;
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
				const size_t glyphIndex = pFontData->GetGlyphIndex(c);
				const GlyphData* glyph = pFontData->GetGlyph(glyphIndex);

				if (!glyph) continue;

				const float xpos = writeX + glyph->Bearing.x * scale;
				const float ypos = writeY - (glyph->Size.y - glyph->Bearing.y) * scale;

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
				pMesh->AddFace(letterCount * 4 + 0, letterCount * 4 + 1, letterCount * 4 + 2, letterCount * 4 + 3);
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

		const float space = spaceGlyph ? (spaceGlyph->Advance >> 6) * scale : 0.0f;
		for (size_t i = 0; i < wordPositions.size(); ++i)
		{
			const std::pair<size_t, size_t>& wordPosition = wordPositions[i];
			const std::string_view word = text.substr(wordPosition.first, wordPosition.second);

			/* Calculate word width */
			float wordWidth = 0.0f;
			for (char c : word)
			{
				const size_t glyphIndex = pFontData->GetGlyphIndex(c);
				const GlyphData* glyph = pFontData->GetGlyph(glyphIndex);

				if (!glyph) continue;
				const float advance = (glyph->Advance >> 6) * scale;
				wordWidth += advance;
			}

			if (currentLineLength > 0 && textWrap > 0.0f && currentLineWidth + wordWidth >= textWrap)
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

	MeshData* UIDocument::GetTextMesh(const TextRenderData& data, FontData* pFont)
	{
		auto iter = m_pTextMeshes.find(data.m_ObjectID);
		const bool exists = iter != m_pTextMeshes.end();
		if (!exists)
		{
			MeshData* pMesh = new MeshData(data.m_Text.size() * 4, sizeof(VertexPosColorTex),
				{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
			iter = m_pTextMeshes.emplace(data.m_ObjectID, pMesh).first;
		}

		if (data.m_TextDirty || !exists)
			GenerateTextMesh(iter->second.get(), pFont, data);
		return iter->second.get();
	}

	void UIDocument::SetRenderTexture(RenderTexture* pTexture)
	{
		m_pUITexture = pTexture;
	}

	Utils::ECS::EntityRegistry& UIDocument::Registry()
	{
		return m_Registry;
	}

	std::string_view UIDocument::Name() const
	{
		return m_Name;
	}

	std::string_view UIDocument::Name(Utils::ECS::EntityID entity) const
	{
		return m_Names.at(entity);
	}

	UUID UIDocument::EntityUUID(Utils::ECS::EntityID entity) const
	{
		return m_UUIds.at(entity);
	}

	Utils::ECS::EntityID UIDocument::EntityID(UUID uuid) const
	{
		return m_Ids.at(uuid);
	}

	Utils::ECS::EntityID UIDocument::CreateEmptyEntity(std::string_view name, UUID uuid)
	{
		Utils::ECS::EntityID entity = m_Registry.CreateEntity();
		m_UUIds.emplace(entity, uuid);
		m_Ids.emplace(uuid, entity);
		m_Names.emplace(entity, name);
		return entity;
	}

	Utils::ECS::EntityID UIDocument::CreateEntity(std::string_view name, UUID uuid)
	{
		Utils::ECS::EntityID entity = m_Registry.CreateEntity<UITransform>();
		m_UUIds.emplace(entity, uuid);
		m_Ids.emplace(uuid, entity);
		m_Names.emplace(entity, name);
		return entity;
	}

	void UIDocument::DestroyEntity(UUID uuid)
	{
		const Utils::ECS::EntityID entity = m_Ids.at(uuid);
		for (size_t i = 0; i < m_Registry.ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(entity, i);
			const UUID uuid = m_UUIds.at(child);
			DestroyEntity(uuid);
		}

		m_Registry.DestroyEntity(entity);
		m_Ids.erase(uuid);
		m_UUIds.erase(entity);
		m_Names.erase(entity);
	}

	bool UIDocument::EntityExists(UUID uuid)
	{
		return m_Ids.find(uuid) != m_Ids.end();
	}
}
