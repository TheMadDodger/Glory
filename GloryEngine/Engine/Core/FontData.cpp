#include "FontData.h"
#include "BinaryStream.h"
#include "AssetManager.h"
#include "TextureData.h"
#include "Engine.h"

namespace Glory
{
	FontData::FontData(): m_FontHeight(0), m_Texture(0), m_Material(0)
	{
		APPEND_TYPE(FontData);
	}
	
	FontData::FontData(uint32_t height, std::vector<uint64_t>&& characterCodes,
		std::vector<GlyphData>&& chars):
		m_FontHeight(height), m_CharacterCodes(std::move(characterCodes)),
		m_Glyphs(std::move(chars)), m_Texture(0), m_Material(0)
	{
		APPEND_TYPE(FontData);
	}

	FontData::~FontData()
	{
		m_Glyphs.clear();
	}

	uint32_t FontData::FontHeight() const
	{
		return m_FontHeight;
	}

	size_t FontData::GetGlyphIndex(uint64_t c) const
	{
		auto iter = std::find(m_CharacterCodes.begin(), m_CharacterCodes.end(), c);
		if (iter == m_CharacterCodes.end()) return m_CharacterCodes.size();
		return iter - m_CharacterCodes.begin();
	}

	const GlyphData* FontData::GetGlyph(size_t index) const
	{
		if (index >= m_Glyphs.size()) return nullptr;
		return &m_Glyphs[index];
	}

	TextureData* FontData::GetGlyphTexture(AssetManager& assets) const
	{
		Resource* pResource = assets.FindResource(m_Texture);
		return pResource ? static_cast<TextureData*>(pResource) : nullptr;
	}

	void FontData::SetTexture(UUID texture)
	{
		m_Texture = texture;
	}

	void FontData::SetMaterial(UUID material)
	{
		m_Material = material;
	}

	UUID FontData::Texture()
	{
		return m_Texture;
	}

	UUID FontData::Material()
	{
		return m_Material;
	}

	void FontData::Serialize(BinaryStream& container) const
	{
		container.Write(m_FontHeight).Write(m_Glyphs).
			Write(m_Texture).Write(m_Material);
	}

	void FontData::Deserialize(BinaryStream& container)
	{
		container.Read(m_FontHeight).Read(m_Glyphs).
			Read(m_Texture).Read(m_Material);
		m_CharacterCodes.resize(m_Glyphs.size());
		for (size_t i = 0; i < m_CharacterCodes.size(); ++i)
			m_CharacterCodes[i] = m_Glyphs[i].Code;
	}

	void FontData::References(Engine* pEngine, std::vector<UUID>& references) const
	{
		if (m_Texture)
		{
			references.push_back(m_Texture);
			Resource* pTextureResource = pEngine->GetAssetManager().FindResource(m_Texture);
			if (pTextureResource) pTextureResource->References(pEngine, references);
		}
		if (m_Material)
		{
			references.push_back(m_Material);
			Resource* pMaterialResource = pEngine->GetAssetManager().FindResource(m_Material);
			if (pMaterialResource) pMaterialResource->References(pEngine, references);
		}
	}
}
