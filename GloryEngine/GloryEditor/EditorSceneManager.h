#pragma once
#include <GScene.h>

namespace Glory::Editor
{
	class EditorSceneManager
	{
	public:
		static GScene* NewScene(bool additive = false);
		static void OpenScene(UUID uuid, bool additive);
		static void SaveOpenScenes();

		static void CloseScene(UUID uuid);
		static bool IsSceneOpen(UUID uuid);

		static void CloseAll();

		static size_t OpenSceneCount();
		static UUID GetOpenSceneUUID(size_t index);

		static void SaveScene(UUID uuid);
		static void SaveSceneAs(UUID uuid);

	private:
		static void Save(UUID uuid, const std::string& path, bool newScene = false);

	private:
		static std::vector<UUID> m_OpenedSceneIDs;
		static UUID m_CurrentlySavingScene;
	};
}
