#pragma once
#include <ScenesModule.h>
#include <MaterialData.h>
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

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void OnCleanup() override;

		virtual GScene* CreateScene(const std::string& sceneName) override;
		virtual GScene* CreateScene(const std::string& sceneName, UUID uuid) override;

		static EntitySceneObject* CreateDeserializedObject(GScene* pScene, const std::string& name, UUID uuid);

		virtual SceneObject* GetSceneObjectFromObjectID(uint32_t objectID) override;

	private:
		friend class EntitySceneObjectSerializer;
		friend class EntitySceneSerializer;
	};
}
