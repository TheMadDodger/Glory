#pragma once
#include <GScene.h>

namespace Glory::Editor
{
	class EditorSceneManager
	{
	public:
		static void NewScene();
		static void OpenScene(UUID uuid, bool additive);
		static void SaveOpenScenes();

		static void CloseScene(UUID uuid);
		static bool IsSceneOpen(UUID uuid);

		static void CloseAll();

	private:
		static std::vector<UUID> m_OpenedSceneIDs;
	};
}
