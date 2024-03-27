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
		return EditorAssets::GetTextureData("scene");
	}

	void SceneTumbnailGenerator::OnFileDoubleClick(UUID uuid)
	{
		EditorApplication::GetInstance()->GetSceneManager().OpenScene(uuid, true);
	}
}
