#pragma once
#include <GScene.h>
#include <yaml-cpp/yaml.h>
#include <vector>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class EditorSceneManager
	{
	public:
		static GLORY_EDITOR_API GScene* NewScene(bool additive = false);
		static GLORY_EDITOR_API void OpenScene(UUID uuid, bool additive);
		static GLORY_EDITOR_API void OpenScene(GScene* pScene, UUID uuid = 0);
		static GLORY_EDITOR_API void SaveOpenScenes();

		static GLORY_EDITOR_API void CloseScene(UUID uuid);
		static GLORY_EDITOR_API bool IsSceneOpen(UUID uuid);

		static GLORY_EDITOR_API void CloseAll();

		static GLORY_EDITOR_API size_t OpenSceneCount();
		static GLORY_EDITOR_API UUID GetOpenSceneUUID(size_t index);

		static GLORY_EDITOR_API void SaveScene(UUID uuid);
		static GLORY_EDITOR_API void SaveSceneAs(UUID uuid);

		static GLORY_EDITOR_API void SerializeOpenScenes(YAML::Emitter& out);
		static GLORY_EDITOR_API void OpenAllFromNode(YAML::Node& node);

		static GLORY_EDITOR_API void SetActiveScene(GScene* pScene);
		static GLORY_EDITOR_API void SetSceneDirty(GScene* pScene, bool dirty = true);

		static GLORY_EDITOR_API bool IsSceneDirty(GScene* pScene);
		static GLORY_EDITOR_API bool HasUnsavedChanges();

	private:
		static void Save(UUID uuid, const std::string& path, bool newScene = false);

	private:
		static std::vector<UUID> m_OpenedSceneIDs;
		static std::vector<UUID> m_DirtySceneIDs;
		static UUID m_CurrentlySavingScene;
	};
}
