#include "SceneTumbnailGenerator.h"
#include "EditorAssets.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"

#include <Engine.h>

namespace Glory::Editor
{
	SceneTumbnailGenerator::SceneTumbnailGenerator() : m_pSceneTumbnail(nullptr)
	{
	}

	SceneTumbnailGenerator::~SceneTumbnailGenerator()
	{
	}

	TextureData* SceneTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
	{
		if (!m_pSceneTumbnail)
		{
			LoaderModule* pLoader = EditorApplication::GetInstance()->GetEngine()->GetLoaderModule<ImageData>();
			m_pSceneTumbnail = (ImageData*)pLoader->Load("./EditorAssets/Thumb/scene.png");
		}

		return (TextureData*)m_pSceneTumbnail->Subresource(0);
	}

	void SceneTumbnailGenerator::OnFileDoubleClick(UUID uuid)
	{
		EditorSceneManager::OpenScene(uuid, true);
	}
}
