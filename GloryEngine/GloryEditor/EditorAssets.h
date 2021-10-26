#pragma once
#include <ImageLoaderModule.h>
#include <GraphicsModule.h>

namespace Glory::Editor
{
	class EditorAssets
	{
	public:
		static void Initialize();
		static void Destroy();

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
	};
}
