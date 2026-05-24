#pragma once
#include <UUID.h>
#include <Reflection.h>

#include <engine_visibility.h>

#include <string>
#include <vector>
#include <map>
#include <IComponentManager.h>
#include <RegistryFactory.h>
#include <glm/vec3.hpp>

namespace Glory
{
	class IEngine;
	class GScene;
	class Renderer;
	struct UUIDRemapper;

	class SceneManager
	{
	public:
		GLORY_ENGINE_API SceneManager(IEngine* pEngine);
		GLORY_ENGINE_API virtual ~SceneManager();

		GLORY_ENGINE_API void SetRenderer(Renderer* pRenderer);
		GLORY_ENGINE_API Renderer* GetRenderer() const;

		virtual GScene* NewScene(const std::string& name="Empty Scene", bool additive=false) = 0;
		GLORY_ENGINE_API void LoadScene(UUID uuid, bool additive);
		GLORY_ENGINE_API void UnloadScene(UUID uuid);
		GLORY_ENGINE_API void UnloadAllScenes();
		GLORY_ENGINE_API void LoadSceneNextFrame(UUID uuid, bool additive);

		virtual void OnLoadScene(UUID uuid) = 0;
		virtual void OnUnloadScene(GScene* pScene) = 0;
		virtual void OnUnloadAllScenes() = 0;

		GLORY_ENGINE_API Utils::ECS::RegistryFactory& GetRegistryFactory();

		template<Utils::ECS::IsComponentManager Manager, typename Component>
		inline void RegisterComponentManager(std::function<void(Utils::ECS::EntityRegistry*, Manager*)> createCallback=NULL)
		{
			m_RegistryFactory.RegisterComponentManager<Manager>(createCallback);
		    Reflect::RegisterType<Component>();
		}

		/** @brief Get the engine that owns this manager */
		GLORY_ENGINE_API IEngine* GetEngine();
		GLORY_ENGINE_API GScene* GetActiveScene(bool force=false);
		GLORY_ENGINE_API void SetActiveScene(GScene* pScene);

		GLORY_ENGINE_API size_t OpenScenesCount();
		GLORY_ENGINE_API GScene* GetOpenScene(size_t index);
		GLORY_ENGINE_API GScene* GetOpenScene(UUID uuid);
		GLORY_ENGINE_API size_t GetSceneIndex(UUID uuid) const;
		GLORY_ENGINE_API size_t ExternalSceneCount();
		GLORY_ENGINE_API GScene* GetExternalScene(size_t index);

		GLORY_ENGINE_API void UpdateScene(GScene* pScene, float dt) const;
		GLORY_ENGINE_API void DrawScene(GScene* pScene) const;

		GLORY_ENGINE_API void Start();
		GLORY_ENGINE_API void Stop();

		/** @brief Add an external scene */
		GLORY_ENGINE_API void AddExternalScene(GScene* pScene);
		GLORY_ENGINE_API void RemoveExternalScene(GScene* pScene);

		GLORY_ENGINE_API UUID AddSceneClosingCallback(std::function<void(UUID, UUID)> callback);
		GLORY_ENGINE_API void RemoveSceneClosingCallback(UUID id);

		GLORY_ENGINE_API UUID AddSceneObjectDestroyedCallback(std::function<void(UUID, UUID)> callback);
		GLORY_ENGINE_API void RemoveSceneObjectDestroyedCallback(UUID id);

		GLORY_ENGINE_API void OnSceneObjectDestroyed(UUID objectID, UUID sceneID);

		template<typename T>
		inline void SubscribeOnCopy(std::function<void(GScene*, void*, UUID, UUIDRemapper&)> callback)
		{
			const uint32_t hash = T::GetTypeData()->TypeHash();
			SubscribeOnCopy(hash, callback);
		}

		GLORY_ENGINE_API void SubscribeOnCopy(uint32_t hash, std::function<void(GScene*, void*, UUID, UUIDRemapper&)> callback);
		GLORY_ENGINE_API void TriggerOnCopy(uint32_t hash, GScene* pScene, void* data, UUID componentID, UUIDRemapper& remapper);

		GLORY_ENGINE_API void Initialize();
		GLORY_ENGINE_API void Cleanup();
		GLORY_ENGINE_API void Update(float dt);
		GLORY_ENGINE_API void Draw();
		GLORY_ENGINE_API void RegisterComponentManagers();

	protected:
		virtual void OnInitialize() = 0;
		virtual void OnCleanup() = 0;
		virtual void OnSetActiveScene(GScene* pActiveScene) = 0;

		GLORY_ENGINE_API GScene* CreateNewScene_Internal(const std::string& name, UUID uuid=UUID());
		
	protected:
		IEngine* m_pEngine;
		Renderer* m_pRenderer;
		std::vector<GScene*> m_pOpenScenes;
		std::vector<GScene*> m_pExternalScenes;
		std::vector<GScene*> m_pRemovedScenes;
		size_t m_ActiveSceneIndex;
		UUID m_HoveringObjectSceneID;
		UUID m_HoveringObjectID;
		glm::vec3 m_HoveringPos;
		glm::vec3 m_HoveringNormal;

		std::vector<UUID> m_ToLoadNextFrame;
		bool m_NextFrameLoadIsAdditive;

		Glory::Utils::ECS::RegistryFactory m_RegistryFactory;

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
