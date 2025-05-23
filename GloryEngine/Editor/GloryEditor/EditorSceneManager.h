#pragma once
#include "GloryEditor.h"
#include "EditableResource.h"

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
	template<typename T>
	struct Dispatcher;

	class EditorApplication;

	class EditorSceneManager final : public SceneManager
	{
	public:
		EditorSceneManager(EditorApplication* pApplication);
		virtual ~EditorSceneManager();

		/* Scene manager overrides */
		GLORY_EDITOR_API GScene* NewScene(const std::string& name="Empty Scene", bool additive=false);
		GLORY_EDITOR_API void OnLoadScene(UUID uuid) override;
		GLORY_EDITOR_API void OnUnloadScene(GScene* pScene) override;
		GLORY_EDITOR_API void OnUnloadAllScenes() override;

		/* Editor only functionality */
		GLORY_EDITOR_API static GScene* OpenSceneInMemory(UUID uuid);
		GLORY_EDITOR_API void OpenScene(UUID uuid, bool additive);
		GLORY_EDITOR_API void OpenScene(GScene* pScene, UUID uuid = 0);
		GLORY_EDITOR_API void SaveOpenScenes();
		GLORY_EDITOR_API void CloseAllScenes();

		GLORY_EDITOR_API void CloseScene(UUID uuid);

		GLORY_EDITOR_API UUID GetOpenSceneUUID(size_t index);

		GLORY_EDITOR_API void SaveScene(UUID uuid);
		GLORY_EDITOR_API void SaveSceneAs(UUID uuid);

		GLORY_EDITOR_API void SerializeOpenScenes(Utils::InMemoryYAML out);
		GLORY_EDITOR_API void OpenAllFromYAML(Utils::InMemoryYAML data);

		GLORY_EDITOR_API void SetSceneDirty(GScene* pScene, bool dirty = true);

		GLORY_EDITOR_API bool IsSceneDirty(GScene* pScene);
		GLORY_EDITOR_API bool HasUnsavedChanges();

		GLORY_EDITOR_API void DuplicateSceneObject(Entity entity);
		GLORY_EDITOR_API void PasteSceneObject(GScene* pScene, Utils::ECS::EntityID parent, Utils::NodeValueRef entities);

		GLORY_EDITOR_API YAMLResource<GScene>* GetSceneFile(UUID uuid);

		GLORY_EDITOR_API static void SetupCallbacks(GScene* pScene);

		enum SceneEventType
		{
			Opened,
			Closed
		};
		struct EditorSceneEvent
		{
			SceneEventType Type;
			UUID SceneID;
		};

		using SceneEventDispatcher = Dispatcher<EditorSceneEvent>;
		GLORY_EDITOR_API SceneEventDispatcher& SceneEventsDispatcher();

	private:
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
		virtual void OnSetActiveScene(GScene* pActiveScene) override;

	private:
		void Save(UUID uuid, const std::string& path, bool newScene = false);

	private:
		std::vector<UUID> m_OpenedSceneIDs;
		std::vector<UUID> m_DirtySceneIDs;
		std::vector<YAMLResource<GScene>> m_SceneFiles;
		UUID m_CurrentlySavingScene = 0;

		EditorApplication* m_pApplication;
	};
}
