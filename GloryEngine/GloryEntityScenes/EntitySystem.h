//#pragma once
//#include <typeindex>
//#include <vector>
//#include <SerializedProperty.h>
//#include <Glory.h>
//#include "EntityComponentData.h"
//
//namespace Glory
//{
//	class Registry;
//
//	class EntitySystem
//	{
//	public:
//		GLORY_API EntitySystem(Registry* pRegistry, const std::type_index& type);
//		GLORY_API virtual ~EntitySystem();
//
//		GLORY_API virtual void ComponentAdded(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) = 0;
//		GLORY_API virtual void ComponentRemoved(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) = 0;
//
//		GLORY_API virtual void Update(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) = 0;
//		GLORY_API virtual void Draw(Registry* pRegistry, EntityID entity, EntityComponentData* pComponentData) = 0;
//		GLORY_API virtual std::string AcquireSerializedProperties(UUID uuid, EntityComponentData* pComponentData, std::vector<SerializedProperty*>& properties) = 0;
//		GLORY_API virtual void CreateComponent(EntityID entity, UUID uuid) = 0;
//
//		GLORY_API bool Hidden();
//		GLORY_API virtual std::string GetPath();
//		GLORY_API virtual std::string Name();
//
//		GLORY_API std::type_index GetComponentType() const;
//
//	protected:
//		friend class EntitySystems;
//		Registry* m_pRegistry;
//		std::type_index m_ComponentType;
//		bool m_Hidden = false;
//	};
//}
