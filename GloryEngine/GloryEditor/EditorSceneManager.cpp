#include "EditorSceneManager.h"
#include <Game.h>
#include <Engine.h>
#include <ScenesModule.h>
#include <AssetDatabase.h>
#include <Serializer.h>

namespace Glory::Editor
{
	std::vector<UUID> EditorSceneManager::m_OpenedSceneIDs;

	void EditorSceneManager::NewScene()
	{
		CloseAll();
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		GScene* pScene = pScenesModule->CreateEmptyScene();
		m_OpenedSceneIDs.push_back(pScene->GetUUID());
	}

	void EditorSceneManager::OpenScene(UUID uuid, bool additive)
	{
		if (additive && IsSceneOpen(uuid))
			CloseScene(uuid);
		if (!additive) CloseAll();

		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		AssetLocation location;
		AssetDatabase::GetAssetLocation(uuid, location);
		std::string path = Game::GetGame().GetAssetPath() + "\\" + location.m_Path;
		pScenesModule->OpenScene(path, uuid);
		m_OpenedSceneIDs.push_back(uuid);
	}

	void EditorSceneManager::SaveOpenScenes()
	{
		std::for_each(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), [](UUID uuid)
		{
			AssetLocation location;
			if (!AssetDatabase::GetAssetLocation(uuid, location)) // new scene
			{
				// TODO: Need to bring up a file browser here
				return;
			}

			GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(uuid);
			YAML::Emitter out;
			Serializer::SerializeObject(pScene, out);
			std::string path = Game::GetGame().GetAssetPath() + "\\" + location.m_Path;
			std::ofstream outStream(path);
			outStream << out.c_str();
			outStream.close();
		});
	}

	void EditorSceneManager::CloseScene(UUID uuid)
	{
		// TODO: Check if scene has changes
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		pScenesModule->CloseScene(uuid);
		auto it = std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
		if (it == m_OpenedSceneIDs.end()) return;
		m_OpenedSceneIDs.erase(it);
	}

	bool EditorSceneManager::IsSceneOpen(UUID uuid)
	{
		auto it = std::find(m_OpenedSceneIDs.begin(), m_OpenedSceneIDs.end(), uuid);
		return it != m_OpenedSceneIDs.end();
	}

	void EditorSceneManager::CloseAll()
	{
		m_OpenedSceneIDs.clear();
		ScenesModule* pScenesModule = Game::GetGame().GetEngine()->GetScenesModule();
		pScenesModule->CloseAllScenes();
	}
}
