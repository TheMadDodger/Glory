#pragma once
#include "UUID.h"

#include <string>
#include <vector>
#include <map>
#include <ComponentTypes.h>

namespace Glory
{
	class Engine;
	class GScene;

	class SceneManager
	{
	public:
		GScene* CreateEmptyScene(const std::string& name = "Empty Scene");

		size_t OpenScenesCount();
		GScene* GetOpenScene(size_t index);
		GScene* GetOpenScene(UUID uuid);
		GScene* GetActiveScene();
		void SetActiveScene(GScene* pScene);
		void CloseAllScenes();
		void AddOpenScene(GScene* pScene, UUID uuid = 0);
		void CloseScene(UUID uuid);

		Utils::ECS::ComponentTypes* ComponentTypesInstance() const;

		template<typename T>
		void RegisterComponent(bool allowMultiple = false, const uint64_t customFlags = 0)
		{
		    Utils::ECS::ComponentTypes::RegisterComponent<T>(allowMultiple, customFlags);
		    Reflect::RegisterType<T>();
		}

		GScene* GetHoveringEntityScene();
		UUID GetHoveringEntityUUID() const;
		void SetHoveringObject(UUID sceneID, UUID objectID);

		/** @brief Get the engine that owns this manager */
		Engine* GetEngine();

	private:
		friend class Engine;
		SceneManager(Engine* pEngine);
		virtual ~SceneManager();

	private:
		void Initialize();
		void Cleanup();
		void Update();
		void Draw();

		void OnSceneOpen(UUID uuid) {}
		void OnSceneClose(UUID uuid) {}
		
	protected:
		Engine* m_pEngine;
		std::vector<GScene*> m_pOpenScenes;
		size_t m_ActiveSceneIndex;
		UUID m_HoveringObjectSceneID;
		UUID m_HoveringObjectID;

		Glory::Utils::ECS::ComponentTypes* m_pComponentTypesInstance;
	};
}
