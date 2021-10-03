#pragma once
#include <vector>
#include <JobManager.h>
#include "Registry.h"

namespace Glory
{
	class Entity;

	class EntityScene
	{
	public:
		EntityScene();
		virtual ~EntityScene();

		Entity& CreateEntity();

	private:
		friend class Entity;
		friend class EntitySceneScenesModule;
		std::vector<Entity> m_Entities;
		Registry m_Registry;
	};
}
