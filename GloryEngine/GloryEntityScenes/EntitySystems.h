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

		bool IsUpdating();

	private:
		void OnUpdate();
		void OnDraw();

		template<typename C, typename T>
		EntitySystem* Register(Registry* pRegistry)
		{
			T* pSystem = new T(pRegistry, typeid(C));
			m_pEntitySystems.push_back((EntitySystem*)pSystem);
			return pSystem;
		}

		template<typename T>
		EntitySystem* Register(Registry* pRegistry)
		{
			T* pSystem = new T(pRegistry);
			m_pEntitySystems.push_back((EntitySystem*)pSystem);
			return pSystem;
		}

	private:
		friend class Registry;
		std::vector<EntitySystem*> m_pEntitySystems;
		//Jobs::JobPool<int>* m_pJobPool;
	};
}
