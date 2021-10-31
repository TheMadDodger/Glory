#pragma once
#include <ImageLoaderModule.h>
#include <GraphicsModule.h>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace Glory::Editor
{
	class EditorAssets
	{
	public:
		static void LoadAssets();
		static void Destroy();
		static void EnqueueTextureCreation(ImageData* pImage);

		static Texture* GetTexture(const std::string& key);

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
