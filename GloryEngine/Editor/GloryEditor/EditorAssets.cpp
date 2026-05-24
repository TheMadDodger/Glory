#include "EditorAssets.h"
#include "EditorApplication.h"
#include "Importer.h"
#include "TextureData.h"
#include "EditorTextureData.h"

#include <mutex>
#include <unordered_map>
#include <vector>

#include <IEngine.h>
#include <GraphicsDevice.h>

namespace Glory::Editor
{
	bool EditorAssets::m_IsInitialized = false;
	std::unordered_map<std::string, EditorTextureData*> TextureDatas;
	std::unordered_map<std::string, TextureHandle> Textures;
	std::mutex QueueLock;
	std::vector<TextureData*> TextureCreationQueue;

	static constexpr std::string_view ThumbRootPath = "./EditorAssets/Thumb";

	void EditorAssets::LoadAssets()
	{
		GraphicsDevice* pDevice = EditorApplication::GetInstance()->GetEngine()->ActiveGraphicsDevice();

		std::unique_lock<std::mutex> lock(QueueLock);
		for (size_t i = 0; i < TextureCreationQueue.size(); i++)
			pDevice->AcquireCachedTexture(TextureCreationQueue[i]);
		TextureCreationQueue.clear();
		lock.unlock();

		if (m_IsInitialized) return;
		LoadDirectory(pDevice, ThumbRootPath);
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

	TextureHandle EditorAssets::GetTexture(const std::string& key)
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

	void EditorAssets::LoadDirectory(GraphicsDevice* pDevice, const std::filesystem::path& path)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_directory())
			{
				LoadDirectory(pDevice, entry.path());
				continue;
			}
			const std::filesystem::path filePath = entry.path();
			const bool isImage = filePath.extension().compare(".png") == 0 ||
				filePath.extension().compare(".jpg") == 0 ||
				filePath.extension().compare(".jpeg") == 0;
			if (!isImage) continue;

			std::filesystem::path relative = filePath.lexically_relative(ThumbRootPath);
			relative.replace_extension("");
			std::stringstream str;
			for (const auto& pathPiece : relative)
			{
				if (!str.str().empty())
					str << "/";
				str << pathPiece.string();
			}
			LoadImageAsset(pDevice, filePath, str.str());
		}
	}

	void EditorAssets::LoadImageAsset(GraphicsDevice* pDevice, const std::filesystem::path& path, const std::string& key)
	{
		ImportedResource resource = Importer::Import(path, nullptr);

		ImageData* pImageData = (ImageData*)*resource;
		resource.Cleanup();
		EditorTextureData* pTextureData = new EditorTextureData(pImageData);
		TextureDatas.emplace(key, pTextureData);
		TextureHandle texture = pDevice->AcquireCachedTexture(pTextureData);
		Textures.emplace(key, texture);
	}

	EditorAssets::EditorAssets() {}
	EditorAssets::~EditorAssets() {}

	EditorAssetNames::EditorAssetNames() {}
	EditorAssetNames::~EditorAssetNames() {}
}