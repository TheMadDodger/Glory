#include "EditorAssets.h"
#include <Game.h>

namespace Glory::Editor
{
	bool EditorAssets::m_IsInitialized = false;
	std::vector<ImageData*> EditorAssets::m_pEditorImages;
	std::unordered_map<std::string, Texture*> EditorAssets::m_pTextures;
	std::mutex EditorAssets::m_QueueLock;
	std::vector<ImageData*> EditorAssets::m_pTextureCreationQueue;

	void EditorAssets::LoadAssets()
	{
		GraphicsModule* pGraphics = Game::GetGame().GetEngine()->GetGraphicsModule();

		std::unique_lock<std::mutex> lock(m_QueueLock);
		for (size_t i = 0; i < m_pTextureCreationQueue.size(); i++)
		{
			pGraphics->GetResourceManager()->CreateTexture(m_pTextureCreationQueue[i]);
		}
		m_pTextureCreationQueue.clear();
		lock.unlock();

		if (m_IsInitialized) return;

		LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule<ImageData>();
		LoadImage(pGraphics, pLoader, "./EditorAssets/back.png", "back");
		LoadImage(pGraphics, pLoader, "./EditorAssets/forward.png", "forward");
		LoadImage(pGraphics, pLoader, "./EditorAssets/refresh.png", "refresh");
		LoadImage(pGraphics, pLoader, "./EditorAssets/up.png", "up");
		LoadImage(pGraphics, pLoader, "./EditorAssets/folder.png", "folder");
		LoadImage(pGraphics, pLoader, "./EditorAssets/file.png", "file");
		LoadImage(pGraphics, pLoader, "./EditorAssets/scene.png", "scene");
		m_IsInitialized = true;
	}

	void EditorAssets::Destroy()
	{
		for (size_t i = 0; i < m_pEditorImages.size(); i++)
		{
			delete m_pEditorImages[i];
		}
		m_pEditorImages.clear();
		m_pTextures.clear();
	}

	void EditorAssets::EnqueueTextureCreation(ImageData* pImage)
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
		ImageData* pImageData = (ImageData*)pLoader->Load(path);
		m_pEditorImages.push_back(pImageData);
		m_pTextures[key] = pGraphics->GetResourceManager()->CreateTexture(pImageData);
	}

	EditorAssets::EditorAssets() {}
	EditorAssets::~EditorAssets() {}
}