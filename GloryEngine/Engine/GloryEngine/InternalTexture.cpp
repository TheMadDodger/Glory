#include "InternalTexture.h"

#include <BinaryStream.h>

namespace Glory
{
	InternalTexture::InternalTexture():
		m_pImage(nullptr), m_Owning(false)
	{
		APPEND_TYPE(InternalTexture);
	}

	InternalTexture::InternalTexture(ImageData* pImage, bool owning):
		m_pImage(pImage), m_Owning(owning)
	{
		APPEND_TYPE(InternalTexture);
	}

	InternalTexture::~InternalTexture()
	{
		if (!m_pImage || !m_Owning) return;
		delete m_pImage;
		m_pImage = nullptr;
	}

	ImageData* InternalTexture::GetImageData(Resources* pAssetManager)
	{
		return m_pImage;
	}

	void InternalTexture::Serialize(Utils::BinaryStream& container) const
	{
		TextureData::Serialize(container);
		container.Write(m_pImage ? 1 : 0);
		if (m_pImage)
			m_pImage->Serialize(container);
	}

	void InternalTexture::Deserialize(Utils::BinaryStream& container)
	{
		TextureData::Deserialize(container);
		int hasImage;
		container.Read(hasImage);
		if (hasImage)
		{
			if (!m_pImage)
				m_pImage = new ImageData();
			m_pImage->Deserialize(container);
		}
	}

	void InternalTexture::SetImage(ImageData* pImage, bool owning)
	{
		if (m_pImage && m_Owning)
			delete m_pImage;

		m_pImage = pImage;
		m_Owning = owning;
	}
}
