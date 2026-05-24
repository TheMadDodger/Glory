#pragma once
#include "GloryEditor.h"

#include <GraphicsHandles.h>
#include <string>
#include <filesystem>

namespace Glory
{
	class TextureData;
	class GraphicsDevice;
}

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
		static GLORY_EDITOR_API void EnqueueTextureCreation(TextureData* pImage);

		static GLORY_EDITOR_API TextureHandle GetTexture(const std::string& key);
		static GLORY_EDITOR_API TextureData* GetTextureData(const std::string& key);

	private:
		static void LoadDirectory(GraphicsDevice* pDevice, const std::filesystem::path& path);
		static void LoadImageAsset(GraphicsDevice* pDevice, const std::filesystem::path& path, const std::string& key);

		EditorAssets();
		virtual ~EditorAssets();

	private:
		static bool m_IsInitialized;
	};
}
