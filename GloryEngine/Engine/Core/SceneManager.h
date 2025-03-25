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
	struct UUIDRemapper;

	class SceneManager
	{
	public:
		SceneManager(Engine* pEngine);
		virtual ~SceneManager();

		virtual GScene* NewScene(const std::string& name="Empty Scene", bool additive=false) = 0;
		void LoadScene(UUID uuid, bool additive);
		void UnloadScene(UUID uuid);
		void UnloadAllScenes();

		virtual void OnLoadScene(UUID uuid) = 0;
		virtual void OnUnloadScene(GScene* pScene) = 0;
		virtual void OnUnloadAllScenes() = 0;

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
		const glm::vec3& GetHoveringNormal() const;
		void SetHoveringObject(UUID sceneID, UUID objectID);
		void SetHoveringPosition(const glm::vec3& pos);
		void SetHoveringNormal(const glm::vec3& normal);

		/** @brief Get the engine that owns this manager */
		Engine* GetEngine();
		GScene* GetActiveScene(bool force=false);
		void SetActiveScene(GScene* pScene);

		size_t OpenScenesCount();
		GScene* GetOpenScene(size_t index);
		GScene* GetOpenScene(UUID uuid);
		size_t GetSceneIndex(UUID uuid) const;
		size_t ExternalSceneCount();
		GScene* GetExternalScene(size_t index);

		void UpdateScene(GScene* pScene) const;
		void DrawScene(GScene* pScene) const;

		void Start();
		void Stop();

		/** @brief Add an external scene */
		void AddExternalScene(GScene* pScene);
		void RemoveExternalScene(GScene* pScene);

		UUID AddSceneClosingCallback(std::function<void(UUID, UUID)> callback);
		void RemoveSceneClosingCallback(UUID id);

		UUID AddSceneObjectDestroyedCallback(std::function<void(UUID, UUID)> callback);
		void RemoveSceneObjectDestroyedCallback(UUID id);

		void OnSceneObjectDestroyed(UUID objectID, UUID sceneID);

		template<typename T>
		void SubscribeOnCopy(std::function<void(GScene*, void*, UUID, UUIDRemapper&)> callback)
		{
			const uint32_t hash = T::GetTypeData()->TypeHash();
			SubscribeOnCopy(hash, callback);
		}

		void SubscribeOnCopy(uint32_t hash, std::function<void(GScene*, void*, UUID, UUIDRemapper&)> callback);
		void TriggerOnCopy(uint32_t hash, GScene* pScene, void* data, UUID componentID, UUIDRemapper& remapper);

	protected:
		virtual void OnInitialize() = 0;
		virtual void OnCleanup() = 0;
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
		std::vector<GScene*> m_pRemovedScenes;
		size_t m_ActiveSceneIndex;
		UUID m_HoveringObjectSceneID;
		UUID m_HoveringObjectID;
		glm::vec3 m_HoveringPos;
		glm::vec3 m_HoveringNormal;

		Glory::Utils::ECS::ComponentTypes* m_pComponentTypesInstance;

		struct SceneCallback
		{
			UUID m_CallbackID;
			std::function<void(UUID, UUID)> m_Callback;
		};
		std::vector<SceneCallback> m_SceneClosedCallbacks;
		std::vector<SceneCallback> m_SceneObjectDestroyedCallbacks;
		std::map<uint32_t, std::function<void(GScene*, void*, UUID, UUIDRemapper&)>> m_OnComponentCopyCallbacks;
	};
}
