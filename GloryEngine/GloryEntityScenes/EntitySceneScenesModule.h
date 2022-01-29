#pragma once
#include <ScenesModule.h>
#include "EntityScene.h"
#include "Entity.h"
#include "EntitySceneObject.h"
#include <MaterialData.h>

namespace Glory
{
	class EntitySceneScenesModule : public ScenesModule
	{
	public:
		EntitySceneScenesModule();
		virtual ~EntitySceneScenesModule();

		MaterialData* m_pMaterialData;

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void OnCleanup() override;

		virtual GScene* CreateScene(const std::string& sceneName) override;
		virtual GScene* CreateScene(const std::string& sceneName, UUID uuid) override;

		static EntitySceneObject* CreateDeserializedObject(GScene* pScene, const std::string& name, UUID uuid);

	private:
		friend class EntitySceneObjectSerializer;
	};
}
