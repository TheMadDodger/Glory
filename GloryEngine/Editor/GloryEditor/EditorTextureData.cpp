#include "EditorTextureData.h"

namespace Glory::Editor
{
	EditorTextureData::EditorTextureData(ImageData* pImageData): m_pImageData(pImageData)
	{
	}

	EditorTextureData::~EditorTextureData()
	{
		if (!m_pImageData) return;
		delete m_pImageData;
		m_pImageData = nullptr;
	}

	ImageData* EditorTextureData::GetImageData(AssetManager*)
	{
		return m_pImageData;
	}
}
