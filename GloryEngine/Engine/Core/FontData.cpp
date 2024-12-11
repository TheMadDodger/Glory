#include "FontData.h"
#include "BinaryStream.h"
#include "InternalTexture.h"

namespace Glory
{
	FontData::FontData()
	{
		APPEND_TYPE(FontData);
	}
	
	FontData::FontData(std::vector<Character>&& chars, std::vector<InternalTexture*>&& textures):
		m_Characters(std::move(chars)), m_Textures(textures)
	{
		APPEND_TYPE(FontData);
	}

	FontData::~FontData()
	{
		for (size_t i = 0; i < m_Textures.size(); ++i)
		{
			delete m_Textures[i];
		}
		m_Characters.clear();
		m_Textures.clear();
	}

	const Character* FontData::GetCharacter(char c) const
	{
		if (size_t(c) > m_Characters.size()) return nullptr;
		return &m_Characters[c];
	}

	InternalTexture* FontData::GetCharacterTexture(char c) const
	{
		if (size_t(c) > m_Characters.size()) return nullptr;
		return m_Textures[c];
	}

	void FontData::Serialize(BinaryStream& container) const
	{
		container.Write(m_Characters).Write(m_Textures.size());
		for (size_t i = 0; i < m_Textures.size(); ++i)
		{
			m_Textures[i]->Serialize(container);
		}
	}

	void FontData::Deserialize(BinaryStream& container)
	{
		size_t numTextures;
		container.Read(m_Characters).Read(numTextures);
		m_Textures.resize(numTextures);
		for (size_t i = 0; i < m_Textures.size(); ++i)
		{
			if (!m_Textures[i]) m_Textures[i] = new InternalTexture();
			m_Textures[i]->Deserialize(container);
		}
	}
}
