#include "SceneTumbnailGenerator.h"
#include "EditorAssets.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"
#include "Importer.h"

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
			m_pSceneTumbnail = (ImageData*)Importer::Import("./EditorAssets/Thumb/scene.png", nullptr);
		}

		return (TextureData*)m_pSceneTumbnail->Subresource(0);
	}

	void SceneTumbnailGenerator::OnFileDoubleClick(UUID uuid)
	{
		EditorSceneManager::OpenScene(uuid, true);
	}
}
