#include "EditorAssets.h"
#include <Game.h>

namespace Glory::Editor
{
	bool EditorAssets::m_IsInitialized = false;
	std::vector<ImageData*> EditorAssets::m_pEditorImages;
	std::unordered_map<std::string, Texture*> EditorAssets::m_pTextures;

	void EditorAssets::Initialize()
	{
		if (m_IsInitialized) return;

		GraphicsModule* pGraphics = Game::GetGame().GetEngine()->GetGraphicsModule();
		LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule<ImageData>();
		LoadImage(pGraphics, pLoader, "./EditorAssets/back.png", "back");
		LoadImage(pGraphics, pLoader, "./EditorAssets/forward.png", "forward");
		LoadImage(pGraphics, pLoader, "./EditorAssets/refresh.png", "refresh");
		LoadImage(pGraphics, pLoader, "./EditorAssets/up.png", "up");
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