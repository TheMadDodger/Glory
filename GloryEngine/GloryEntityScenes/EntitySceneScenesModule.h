#pragma once
#include <ScenesModule.h>
#include "EntityScene.h"
#include "Entity.h"

namespace Glory
{
	class EntitySceneScenesModule : public ScenesModule
	{
	public:
		EntitySceneScenesModule();
		virtual ~EntitySceneScenesModule();

	private:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void OnCleanup() override;

		virtual GScene* CreateScene(const std::string& sceneName) override;
		virtual GScene* CreateScene(const std::string& sceneName, UUID uuid) override;

	private:
		Entity m_Entity;
		EntityScene m_Scene;
	};
}
