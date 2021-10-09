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
		virtual void Cleanup() override;
		virtual void Tick() override;
		virtual void OnPaint() override;

	private:
		EntityScene m_Scene;
		Entity m_Entity;
	};
}
