#include "EditorAssets.h"
#include "EditorApplication.h"
#include "Importer.h"
#include "EditorTextureData.h"

#include <mutex>
#include <unordered_map>
#include <vector>
#include <Engine.h>

namespace Glory::Editor
{
	bool EditorAssets::m_IsInitialized = false;
	std::unordered_map<std::string, EditorTextureData*> TextureDatas;
	std::unordered_map<std::string, Texture*> Textures;
	std::mutex QueueLock;
	std::vector<TextureData*> TextureCreationQueue;

	void EditorAssets::LoadAssets()
	{
		GraphicsModule* pGraphics = EditorApplication::GetInstance()->GetEngine()->GetMainModule<GraphicsModule>();

		std::unique_lock<std::mutex> lock(QueueLock);
		for (size_t i = 0; i < TextureCreationQueue.size(); i++)
		{
			pGraphics->GetResourceManager()->CreateTexture(TextureCreationQueue[i]);
		}
		TextureCreationQueue.clear();
		lock.unlock();

		if (m_IsInitialized) return;

		LoadImage(pGraphics, "./EditorAssets/Thumb/folder.png", "folder");
		LoadImage(pGraphics, "./EditorAssets/Thumb/file.png", "file");
		LoadImage(pGraphics, "./EditorAssets/Thumb/scene.png", "scene");
		LoadImage(pGraphics, "./EditorAssets/Thumb/audio.png", "audio");

		m_IsInitialized = true;
	}

	void EditorAssets::Destroy()
	{
		for (auto itor : TextureDatas)
		{
			delete itor.second;
		}

		TextureDatas.clear();
		Textures.clear();
	}

	void EditorAssets::EnqueueTextureCreation(TextureData* pImage)
	{
		std::unique_lock<std::mutex> lock(QueueLock);
		TextureCreationQueue.push_back(pImage);
		lock.unlock();
	}

	Texture* EditorAssets::GetTexture(const std::string& key)
	{
		auto it = Textures.find(key);
		if (it == Textures.end()) return nullptr;
		return it->second;
	}

	TextureData* EditorAssets::GetTextureData(const std::string& key)
	{
		auto it = TextureDatas.find(key);
		if (it == TextureDatas.end()) return nullptr;
		return it->second;
	}

	void EditorAssets::LoadImage(GraphicsModule* pGraphics, const std::string& path, const std::string& key)
	{
		ImportedResource resource = Importer::Import(path, nullptr);

		ImageData* pImageData = (ImageData*)*resource;
		resource.Cleanup();
		EditorTextureData* pTextureData = new EditorTextureData(pImageData);
		TextureDatas.emplace(key, pTextureData);
		Texture* pTexture = pGraphics->GetResourceManager()->CreateTexture(pTextureData);
		Textures.emplace(key, pTexture);
	}

	EditorAssets::EditorAssets() {}
	EditorAssets::~EditorAssets() {}

	EditorAssetNames::EditorAssetNames() {}
	EditorAssetNames::~EditorAssetNames() {}
}