#include "GScene.h"

namespace Glory
{
	GScene::GScene() : m_SceneName("New Scene")
	{
	}

	GScene::GScene(const std::string& sceneName) : m_SceneName(sceneName)
	{
	}

	GScene::GScene(const std::string& sceneName, UUID uuid) : Resource(uuid), m_SceneName(sceneName)
	{
	}

	GScene::~GScene()
	{
	}
}