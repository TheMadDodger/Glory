#include "InternalTexture.h"
#include "BinaryStream.h"

namespace Glory
{
	InternalTexture::InternalTexture():
		m_pImage(nullptr)
	{
		APPEND_TYPE(InternalTexture);
	}

	InternalTexture::InternalTexture(ImageData* pImage):
		m_pImage(pImage)
	{
		APPEND_TYPE(InternalTexture);
	}

	InternalTexture::~InternalTexture()
	{
		if (!m_pImage) return;
		delete m_pImage;
		m_pImage = nullptr;
	}

	ImageData* InternalTexture::GetImageData(AssetManager* pAssetManager)
	{
		return m_pImage;
	}

	void InternalTexture::Serialize(BinaryStream& container) const
	{
		TextureData::Serialize(container);
		container.Write(m_pImage ? 1 : 0);
		if (m_pImage)
			m_pImage->Serialize(container);
	}

	void InternalTexture::Deserialize(BinaryStream& container)
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
}
