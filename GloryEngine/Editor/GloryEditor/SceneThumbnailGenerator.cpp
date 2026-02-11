#include "SceneThumbnailGenerator.h"
#include "EditorAssets.h"
#include "EditorSceneManager.h"
#include "EditorApplication.h"

#include <Engine.h>

namespace Glory::Editor
{
	SceneThumbnailGenerator::SceneThumbnailGenerator() : m_pSceneThumbnail(nullptr)
	{
	}

	SceneThumbnailGenerator::~SceneThumbnailGenerator()
	{
	}

	TextureData* SceneThumbnailGenerator::GetThumbnail(const ResourceMeta* pResourceType)
	{
		return EditorAssets::GetTextureData("scene");
	}

	void SceneThumbnailGenerator::OnFileDoubleClick(UUID uuid)
	{
		EditorApplication::GetInstance()->GetSceneManager().OpenScene(uuid, true);
	}
}
