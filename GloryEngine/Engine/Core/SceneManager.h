#pragma once
#include "UUID.h"

#include <string>
#include <vector>
#include <map>
#include <ComponentTypes.h>
#include <glm/vec3.hpp>

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
		const glm::vec3& GetHoveringPosition() const;
		void SetHoveringObject(UUID sceneID, UUID objectID);
		void SetHoveringPosition(const glm::vec3& pos);

		/** @brief Get the engine that owns this manager */
		Engine* GetEngine();
		GScene* GetActiveScene(bool force=false);
		void SetActiveScene(GScene* pScene);

		size_t OpenScenesCount();
		GScene* GetOpenScene(size_t index);
		GScene* GetOpenScene(UUID uuid);
		size_t ExternalSceneCount();
		GScene* GetExternalScene(size_t index);
		void MarkAllScenesForDestruct();
		void CloseAllScenes();

		void UpdateScene(GScene* pScene) const;
		void DrawScene(GScene* pScene) const;

		void Start();
		void Stop();
		bool HasStarted() const;

		/** @brief Add an external scene */
		void AddExternalScene(GScene* pScene);
		void RemoveExternalScene(GScene* pScene);

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
		std::vector<GScene*> m_pExternalScenes;
		size_t m_ActiveSceneIndex;
		UUID m_HoveringObjectSceneID;
		UUID m_HoveringObjectID;
		glm::vec3 m_HoveringPos;
		bool m_Started{false};

		Glory::Utils::ECS::ComponentTypes* m_pComponentTypesInstance;
	};
}
