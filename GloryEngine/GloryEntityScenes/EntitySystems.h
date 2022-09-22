#pragma once
#include "EntitySystem.h"
#include <vector>
#include <JobManager.h>
#include <SerializedProperty.h>
#include <Glory.h>

namespace Glory
{
	class Registry;

	class EntitySystems
	{
	public:
		GLORY_API EntitySystems();
		GLORY_API virtual ~EntitySystems();

		GLORY_API bool IsUpdating();
		GLORY_API std::string AcquireSerializedProperties(EntityComponentData* pComponentData, std::vector<SerializedProperty*>& properties);
		GLORY_API bool CreateComponent(EntityID entity, std::type_index type, UUID uuid = UUID());
		GLORY_API bool CreateComponent(EntityID entity, size_t typeHash, UUID uuid = UUID());

		GLORY_API size_t SystemCount();
		GLORY_API EntitySystem* GetSystem(size_t index);

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
