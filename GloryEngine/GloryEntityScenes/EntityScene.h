#pragma once
#include <vector>
#include <JobManager.h>
#include <GScene.h>
#include "Registry.h"

namespace Glory
{
	class Entity;

	class EntityScene : public GScene
	{
	public:
		EntityScene();
		EntityScene(const std::string& sceneName = "New Scene");
		EntityScene(const std::string& sceneName, UUID uuid);
		virtual ~EntityScene();

		Entity& CreateEntity();

	private:
		virtual void Initialize() override;
		virtual void OnTick() override;
		virtual void OnPaint() override;

	private:
		friend class Entity;
		std::vector<Entity> m_Entities;
		Registry m_Registry;
	};
}
