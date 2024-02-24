#include "TextureData.h"
#include "BinaryStream.h"

namespace Glory
{
	TextureData::TextureData() : m_Image(UUID(0)), m_SamplerSettings()
	{
		APPEND_TYPE(TextureData);
	}

	TextureData::TextureData(ImageData* pImageData) : m_Image(pImageData), m_SamplerSettings()
	{
		APPEND_TYPE(TextureData);
	}

	TextureData::~TextureData()
	{
	}

	ImageData* TextureData::GetImageData(AssetManager* pAssetManager)
	{
		return Image() ? Image().Get(pAssetManager) : nullptr;
	}

	AssetReference<ImageData>& TextureData::Image()
	{
		return m_Image;
	}

	SamplerSettings& TextureData::GetSamplerSettings()
	{
		return m_SamplerSettings;
	}

	void TextureData::Serialize(BinaryStream& container) const
	{
		/* Write image ID */
		container.Write(m_Image.AssetUUID()).Write(m_SamplerSettings);
	}

	void TextureData::Deserialize(BinaryStream& container)
	{
		container.Read(*m_Image.AssetUUIDMember()).Read(m_SamplerSettings);
	}
}
