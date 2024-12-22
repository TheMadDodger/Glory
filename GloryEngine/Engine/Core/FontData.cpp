#include "FontData.h"
#include "BinaryStream.h"
#include "InternalTexture.h"

namespace Glory
{
	FontData::FontData(): m_FontHeight(0), m_pTexture(nullptr)
	{
		APPEND_TYPE(FontData);
	}
	
	FontData::FontData(uint32_t height, std::vector<uint64_t>&& characterCodes,
		std::vector<GlyphData>&& chars, InternalTexture* pTexture):
		m_FontHeight(height), m_CharacterCodes(std::move(characterCodes)), m_Glyphs(std::move(chars)), m_pTexture(pTexture)
	{
		APPEND_TYPE(FontData);
	}

	FontData::~FontData()
	{
		m_Glyphs.clear();
		if (m_pTexture)
		{
			delete m_pTexture;
			m_pTexture = nullptr;
		}
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

	InternalTexture* FontData::GetGlyphTexture() const
	{
		return m_pTexture;
	}

	void FontData::Serialize(BinaryStream& container) const
	{
		container.Write(m_FontHeight).Write(m_Glyphs);
		m_pTexture->Serialize(container);
	}

	void FontData::Deserialize(BinaryStream& container)
	{
		container.Read(m_FontHeight).Read(m_Glyphs);
		if (!m_pTexture) m_pTexture = new InternalTexture();
		m_pTexture->Deserialize(container);
		m_CharacterCodes.resize(m_Glyphs.size());
		for (size_t i = 0; i < m_CharacterCodes.size(); ++i)
			m_CharacterCodes[i] = m_Glyphs[i].Code;
	}
}
