#include "TextureData.h"

namespace Glory
{
	TextureData::TextureData() : m_Image(UUID(0)), m_SamplerSettings()
	{
		APPEND_TYPE(TextureData);
	}

	TextureData::TextureData(ImageData* pImageData) : m_Image(pImageData), m_SamplerSettings()
	{
	}

	TextureData::~TextureData()
	{
	}

	AssetReference<ImageData>& TextureData::Image()
	{
		return m_Image;
	}

	SamplerSettings& TextureData::GetSamplerSettings()
	{
		return m_SamplerSettings;
	}
}
