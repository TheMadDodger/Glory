#pragma once
#include <ScenesModule.h>
#include "EntityScene.h"

namespace Glory
{
	class EntitySceneScenesModule : public ScenesModule
	{
	public:
		EntitySceneScenesModule();
		virtual ~EntitySceneScenesModule();

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void Tick() override;
		virtual void OnPaint() override;

	private:
		EntityScene m_Scene;
	};
}
