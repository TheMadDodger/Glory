#pragma once
#include "GloryEditor.h"

#include <SceneManager.h>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <Entity.h>

namespace Glory
{
	class GScene;
}

namespace Glory::Editor
{
	class EditorApplication;

	class EditorSceneManager final : public SceneManager
	{
	public:
		EditorSceneManager(EditorApplication* pApplication);
		virtual ~EditorSceneManager();

		/* Scene manager overrides */
		GLORY_EDITOR_API GScene* NewScene(const std::string& name="Empty Scene", bool additive=false);
		GLORY_EDITOR_API void OpenScene(UUID uuid, bool additive) override;

		/* Editor only functionality */
		GLORY_EDITOR_API static GScene* OpenSceneInMemory(UUID uuid);
		GLORY_EDITOR_API void OpenScene(GScene* pScene, UUID uuid = 0);
		GLORY_EDITOR_API void SaveOpenScenes();

		GLORY_EDITOR_API void CloseScene(UUID uuid);
		GLORY_EDITOR_API bool IsSceneOpen(UUID uuid);

		GLORY_EDITOR_API UUID GetOpenSceneUUID(size_t index);

		GLORY_EDITOR_API void SaveScene(UUID uuid);
		GLORY_EDITOR_API void SaveSceneAs(UUID uuid);

		GLORY_EDITOR_API void SerializeOpenScenes(YAML::Emitter& out);
		GLORY_EDITOR_API void OpenAllFromNode(YAML::Node& node);

		GLORY_EDITOR_API void SetActiveScene(GScene* pScene);
		GLORY_EDITOR_API void SetSceneDirty(GScene* pScene, bool dirty = true);

		GLORY_EDITOR_API bool IsSceneDirty(GScene* pScene);
		GLORY_EDITOR_API bool HasUnsavedChanges();

		GLORY_EDITOR_API void DuplicateSceneObject(Entity entity);
		GLORY_EDITOR_API void PasteSceneObject(GScene* pScene, Utils::ECS::EntityID parent, YAML::Node& node);

	private:
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
		virtual void OnCloseAll() override;
		virtual void OnSetActiveScene(GScene* pActiveScene) override;

	private:
		void Save(UUID uuid, const std::string& path, bool newScene = false);

	private:
		std::vector<UUID> m_OpenedSceneIDs;
		std::vector<UUID> m_DirtySceneIDs;
		UUID m_CurrentlySavingScene = 0;

		EditorApplication* m_pApplication;
	};
}
