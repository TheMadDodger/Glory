#include "TextureData.h"

namespace Glory
{
	TextureData::TextureData() : m_pImage(nullptr)
	{
		APPEND_TYPE(TextureData);
	}

	TextureData::TextureData(ImageData* pImageData) : m_pImage(pImageData)
	{
	}

	TextureData::~TextureData()
	{
		m_pImage = nullptr;
	}

	ImageData* TextureData::Image()
	{
		return m_pImage;
	}
}
