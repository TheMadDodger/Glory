#pragma once
#include "EntitySystem.h"
#include <vector>
#include <JobManager.h>
#include <SerializedProperty.h>

namespace Glory
{
	class Registry;

	class EntitySystems
	{
	public:
		EntitySystems();
		virtual ~EntitySystems();

		bool IsUpdating();
		std::string AcquireSerializedProperties(EntityComponentData* pComponentData, std::vector<SerializedProperty>& properties);
		bool CreateComponent(EntityID entity, std::type_index type);
		bool CreateComponent(EntityID entity, size_t typeHash);

		size_t SystemCount();
		EntitySystem* GetSystem(size_t index);

	private:
		void OnComponentAdded(Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData);
		void OnComponentRemoved(Registry* pRegisrty, EntityID entity, EntityComponentData* pComponentData);
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
