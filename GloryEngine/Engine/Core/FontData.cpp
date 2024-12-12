#include "FontData.h"
#include "BinaryStream.h"
#include "InternalTexture.h"

namespace Glory
{
	FontData::FontData(): m_FontHeight(0)
	{
		APPEND_TYPE(FontData);
	}
	
	FontData::FontData(uint32_t height, std::vector<uint64_t>&& characterCodes,
		std::vector<GlyphData>&& chars, std::vector<InternalTexture*>&& textures):
		m_FontHeight(height), m_CharacterCodes(std::move(characterCodes)), m_Glyphs(std::move(chars)), m_Textures(textures)
	{
		APPEND_TYPE(FontData);
	}

	FontData::~FontData()
	{
		for (size_t i = 0; i < m_Textures.size(); ++i)
		{
			delete m_Textures[i];
		}
		m_Glyphs.clear();
		m_Textures.clear();
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

	InternalTexture* FontData::GetGlyphTexture(size_t index) const
	{
		if (index >= m_Glyphs.size()) return nullptr;
		return m_Textures[index];
	}

	void FontData::Serialize(BinaryStream& container) const
	{
		container.Write(m_FontHeight).Write(m_Glyphs).Write(m_Textures.size());
		for (size_t i = 0; i < m_Textures.size(); ++i)
		{
			m_Textures[i]->Serialize(container);
		}
	}

	void FontData::Deserialize(BinaryStream& container)
	{
		size_t numTextures;
		container.Read(m_FontHeight).Read(m_Glyphs).Read(numTextures);
		m_Textures.resize(numTextures);
		for (size_t i = 0; i < m_Textures.size(); ++i)
		{
			if (!m_Textures[i]) m_Textures[i] = new InternalTexture();
			m_Textures[i]->Deserialize(container);
		}
	}
}
