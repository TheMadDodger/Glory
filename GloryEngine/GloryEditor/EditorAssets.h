#pragma once
#include <ImageLoaderModule.h>
#include <GraphicsModule.h>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class EditorAssetNames
	{
	public:
		

	private:
		EditorAssetNames();
		~EditorAssetNames();
	};

	class EditorAssets
	{
	public:
		static GLORY_EDITOR_API void LoadAssets();
		static GLORY_EDITOR_API void Destroy();
		static GLORY_EDITOR_API void EnqueueTextureCreation(ImageData* pImage);

		static GLORY_EDITOR_API Texture* GetTexture(const std::string& key);

	private:
		static void LoadImage(GraphicsModule* pGraphics, LoaderModule* pLoader, const std::string& path, const std::string& key);

	private:
		EditorAssets();
		virtual ~EditorAssets();

	private:
		static bool m_IsInitialized;
		static std::vector<ImageData*> m_pEditorImages;
		static std::unordered_map<std::string, Texture*> m_pTextures;
		static std::mutex m_QueueLock;
		static std::vector<ImageData*> m_pTextureCreationQueue;
	};
}
