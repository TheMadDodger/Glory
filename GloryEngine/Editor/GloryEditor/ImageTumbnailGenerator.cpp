#include "ImageTumbnailGenerator.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"

namespace Glory::Editor
{
	ImageTumbnailGenerator::ImageTumbnailGenerator()
	{
	}

	ImageTumbnailGenerator::~ImageTumbnailGenerator()
	{
	}

	TextureData* ImageTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
	{
		UUID id = pResourceType->ID();
		if (std::find(m_AlreadyRequestedTumbnails.begin(), m_AlreadyRequestedTumbnails.end(), id) != m_AlreadyRequestedTumbnails.end()) return nullptr;

		if (m_pLoadedImages.find(id) != m_pLoadedImages.end())
		{
			ImageData* pImage = m_pLoadedImages[id];
			m_pLoadedImages.erase(id);
			return (TextureData*)pImage->Subresource(0);
		}

		m_AlreadyRequestedTumbnails.push_back(id);

		EditorApplication::GetInstance()->GetResourceManager().GetAsset(id, [&](Resource* pResource)
		{
			UUID uuid = pResource->GetUUID();
			auto it = std::find(m_AlreadyRequestedTumbnails.begin(), m_AlreadyRequestedTumbnails.end(), uuid);
			m_pLoadedImages[uuid] = (ImageData*)pResource;
			if (it == m_AlreadyRequestedTumbnails.end()) return;
			m_AlreadyRequestedTumbnails.erase(it);
		});
		return nullptr;
	}
}