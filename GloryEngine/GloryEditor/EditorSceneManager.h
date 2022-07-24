#pragma once
#include <GScene.h>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class EditorSceneManager
	{
	public:
		static GLORY_EDITOR_API GScene* NewScene(bool additive = false);
		static GLORY_EDITOR_API void OpenScene(UUID uuid, bool additive);
		static GLORY_EDITOR_API void SaveOpenScenes();

		static GLORY_EDITOR_API void CloseScene(UUID uuid);
		static GLORY_EDITOR_API bool IsSceneOpen(UUID uuid);

		static GLORY_EDITOR_API void CloseAll();

		static GLORY_EDITOR_API size_t OpenSceneCount();
		static GLORY_EDITOR_API UUID GetOpenSceneUUID(size_t index);

		static GLORY_EDITOR_API void SaveScene(UUID uuid);
		static GLORY_EDITOR_API void SaveSceneAs(UUID uuid);

	private:
		static void Save(UUID uuid, const std::string& path, bool newScene = false);

	private:
		static std::vector<UUID> m_OpenedSceneIDs;
		static UUID m_CurrentlySavingScene;
	};
}
