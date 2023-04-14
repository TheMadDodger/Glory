#pragma once
#include <ScenesModule.h>
#include <Glory.h>
#include "EntityScene.h"
#include "Entity.h"
#include "EntitySceneObject.h"

namespace Glory
{
	class EntitySceneScenesModule : public ScenesModule
	{
	public:
		GLORY_API EntitySceneScenesModule();
		GLORY_API virtual ~EntitySceneScenesModule();

		GLORY_API GloryECS::ComponentTypes* ComponentTypesInstance() const;

		template<typename T>
		void RegisterComponent(bool allowMultiple = false, const uint64_t customFlags = 0)
		{
			GloryECS::ComponentTypes::RegisterComponent<T>(allowMultiple, customFlags);
			GloryReflect::Reflect::RegisterType<T>();
		}

		GLORY_MODULE_VERSION_H(0,1,0);

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void OnCleanup() override;

		virtual GScene* CreateScene(const std::string& sceneName) override;
		virtual GScene* CreateScene(const std::string& sceneName, UUID uuid) override;

		static EntitySceneObject* CreateDeserializedObject(GScene* pScene, const std::string& name, UUID uuid);

		virtual SceneObject* GetSceneObjectFromObjectID(uint64_t objectID) override;

	private:
		friend class EntitySceneObjectSerializer;
		friend class EntitySceneSerializer;
		GloryECS::ComponentTypes* m_pComponentTypesInstance;
	};
}
