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
		SceneManager(Engine* pEngine);
		virtual ~SceneManager();

		virtual GScene* NewScene(const std::string& name="Empty Scene", bool additive=false) = 0;
		virtual void OpenScene(UUID uuid, bool additive) = 0;
		virtual void CloseScene(UUID uuid) = 0;

		//void AddOpenScene(GScene* pScene, UUID uuid = 0);

		Utils::ECS::ComponentTypes* ComponentTypesInstance() const;

		template<typename T>
		void RegisterComponent(bool allowMultiple = false, const uint64_t customFlags = 0)
		{
			Utils::ECS::ComponentTypes::SetInstance(m_pComponentTypesInstance);
		    Utils::ECS::ComponentTypes::RegisterComponent<T>(allowMultiple, customFlags);
		    Reflect::RegisterType<T>();
		}

		GScene* GetHoveringEntityScene();
		UUID GetHoveringEntityUUID() const;
		void SetHoveringObject(UUID sceneID, UUID objectID);

		/** @brief Get the engine that owns this manager */
		Engine* GetEngine();
		GScene* GetActiveScene(bool force=false);
		void SetActiveScene(GScene* pScene);

		size_t OpenScenesCount();
		GScene* GetOpenScene(size_t index);
		GScene* GetOpenScene(UUID uuid);
		void MarkAllScenesForDestruct();
		void CloseAllScenes();

		void Start();
		void Stop();

	protected:
		virtual void OnInitialize() = 0;
		virtual void OnCleanup() = 0;
		virtual void OnCloseAll() = 0;
		virtual void OnSetActiveScene(GScene* pActiveScene) = 0;

	private:
		friend class Engine;
		void Initialize();
		void Cleanup();
		void Update();
		void Draw();
		
	protected:
		Engine* m_pEngine;
		std::vector<GScene*> m_pOpenScenes;
		size_t m_ActiveSceneIndex;
		UUID m_HoveringObjectSceneID;
		UUID m_HoveringObjectID;
		bool m_Started{false};

		Glory::Utils::ECS::ComponentTypes* m_pComponentTypesInstance;
	};
}
