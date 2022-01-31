#pragma once
#include <Serializer.h>
#include "EntityComponentObject.h"
#include "EntitySceneObject.h"

namespace Glory
{
	template<typename TComponent>
	class EntityComponentSerializer : public SerializerTemplate<EntityComponentObject>
	{
	public:
		EntityComponentSerializer() {}
		virtual ~EntityComponentSerializer() {}

	protected:
		virtual void Serialize(TComponent& component, YAML::Emitter& out) = 0;
		virtual void Deserialize(TComponent& component, YAML::Node& object) = 0;

	private:
		virtual std::type_index GetSerializedType() override
		{
			return typeid(TComponent);
		}

		virtual void Serialize(EntityComponentObject* pObject, YAML::Emitter& out) override
		{
			Serialize(pObject->GetData<TComponent>(), out);
		}

		virtual Object* Deserialize(Object* pParent, YAML::Node& object) override
		{
			EntitySceneObject* pObject = (EntitySceneObject*)pParent;
			Entity entity = pObject->GetEntityHandle();
			if (entity.HasComponent<TComponent>())
			{
				TComponent& component = entity.GetComponent<TComponent>();
				Deserialize(component, object);
				return nullptr;
			}

			TComponent& component = entity.AddComponent<TComponent>();
			Deserialize(component, object);
		}
	};
}
