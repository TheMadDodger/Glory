#include "EditorAssets.h"
#include "EditorApplication.h"
#include "Importer.h"

#include <Engine.h>

namespace Glory::Editor
{
	bool EditorAssets::m_IsInitialized = false;
	std::vector<ImageData*> EditorAssets::m_pEditorImages;
	std::unordered_map<std::string, Texture*> EditorAssets::m_pTextures;
	std::mutex EditorAssets::m_QueueLock;
	std::vector<TextureData*> EditorAssets::m_pTextureCreationQueue;

	void EditorAssets::LoadAssets()
	{
		GraphicsModule* pGraphics = EditorApplication::GetInstance()->GetEngine()->GetMainModule<GraphicsModule>();

		std::unique_lock<std::mutex> lock(m_QueueLock);
		for (size_t i = 0; i < m_pTextureCreationQueue.size(); i++)
		{
			pGraphics->GetResourceManager()->CreateTexture(m_pTextureCreationQueue[i]);
		}
		m_pTextureCreationQueue.clear();
		lock.unlock();

		if (m_IsInitialized) return;

		LoaderModule* pLoader = EditorApplication::GetInstance()->GetEngine()->GetLoaderModule<ImageData>();
		LoadImage(pGraphics, pLoader, "./EditorAssets/Thumb/folder.png", "folder");
		LoadImage(pGraphics, pLoader, "./EditorAssets/Thumb/file.png", "file");
		LoadImage(pGraphics, pLoader, "./EditorAssets/Thumb/scene.png", "scene");
		LoadImage(pGraphics, pLoader, "./EditorAssets/Thumb/audio.png", "audio");

		m_IsInitialized = true;
	}

	void EditorAssets::Destroy()
	{
		m_pEditorImages.clear();
		m_pTextures.clear();
	}

	void EditorAssets::EnqueueTextureCreation(TextureData* pImage)
	{
		std::unique_lock<std::mutex> lock(m_QueueLock);
		m_pTextureCreationQueue.push_back(pImage);
		lock.unlock();
	}

	Texture* EditorAssets::GetTexture(const std::string& key)
	{
		auto it = m_pTextures.find(key);
		if (it == m_pTextures.end()) return nullptr;
		return m_pTextures[key];
	}

	void EditorAssets::LoadImage(GraphicsModule* pGraphics, LoaderModule* pLoader, const std::string& path, const std::string& key)
	{
		ImageData* pImageData = (ImageData*)Importer::Import(path, nullptr);
		TextureData* pTextureData = (TextureData*)pImageData->Subresource(0);
		m_pEditorImages.push_back(pImageData);
		m_pTextures[key] = pGraphics->GetResourceManager()->CreateTexture(pTextureData);
	}

	EditorAssets::EditorAssets() {}
	EditorAssets::~EditorAssets() {}

	EditorAssetNames::EditorAssetNames() {}
	EditorAssetNames::~EditorAssetNames() {}
}