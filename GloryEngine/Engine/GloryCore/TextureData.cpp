#include "TextureData.h"

namespace Glory
{
	TextureData::TextureData() : m_Image(UUID(0))
	{
		APPEND_TYPE(TextureData);
	}

	TextureData::TextureData(ImageData* pImageData) : m_Image(pImageData)
	{
	}

	TextureData::~TextureData()
	{
	}

	AssetReference<ImageData>& TextureData::Image()
	{
		return m_Image;
	}
}
