#pragma once
#include "EntitySystem.h"
#include <vector>
#include <JobManager.h>

namespace Glory
{
	class Registry;

	class EntitySystems
	{
	public:
		EntitySystems();
		virtual ~EntitySystems();

	private:
		friend class Registry;
		std::vector<EntitySystem*> m_pEntitySystems;
		Jobs::JobPool<int>* m_pJobPool;

		void OnUpdate();
		void OnDraw();

		template<typename C, typename T>
		EntitySystems* Register(Registry* pRegistry)
		{
			T* pSystem = new T(pRegistry, typeid(C));
			m_pEntitySystems.push_back(pSystem);
			return pSystem;
		}
	};
}
