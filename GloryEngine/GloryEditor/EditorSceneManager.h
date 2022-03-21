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
		static void SaveOpenScenesAs();

		static void CloseScene(UUID uuid);
		static bool IsSceneOpen(UUID uuid);

		static void CloseAll();

		static size_t OpenSceneCount();
		static UUID GetOpenSceneUUID(size_t index);

	private:
		static void Save(UUID uuid, const std::string& path);

	private:
		static std::vector<UUID> m_OpenedSceneIDs;
	};
}
