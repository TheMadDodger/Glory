#include "SceneTumbnailGenerator.h"
#include "EditorAssets.h"
#include "AssetDatabase.h"
#include "EditorSceneManager.h"

namespace Glory::Editor
{
	SceneTumbnailGenerator::SceneTumbnailGenerator() : m_pSceneTumbnail(nullptr)
	{
	}

	SceneTumbnailGenerator::~SceneTumbnailGenerator()
	{
	}

	ImageData* SceneTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
	{
		if (!m_pSceneTumbnail)
		{
			LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule<ImageData>();
			m_pSceneTumbnail = (ImageData*)pLoader->Load("./EditorAssets/scene.png");
		}

		return m_pSceneTumbnail;
	}

	void SceneTumbnailGenerator::OnFileDoubleClick(UUID uuid)
	{
		EditorSceneManager::OpenScene(uuid, true);
	}
}
