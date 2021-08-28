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
		void Update();

	private:
		friend class Entity;
		std::vector<Entity> m_Entities;
		Registry m_Registry;
		//Jobs::JobPool<void>* m_pJobPool;
	};
}
