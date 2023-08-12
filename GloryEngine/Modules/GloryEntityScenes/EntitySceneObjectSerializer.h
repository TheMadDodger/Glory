#pragma once
#include <Serializer.h>
#include <Reflection.h>
#include "EntitySceneObject.h"
#include "GloryEntityScenes.h"

namespace Glory
{
	class EntitySceneObjectSerializer : public SerializerTemplate<EntitySceneObject>
	{
	public:
		EntitySceneObjectSerializer();
		virtual ~EntitySceneObjectSerializer();

		static GLORY_API void SerializeComponent(Glory::Utils::ECS::EntityID entityID, Glory::Utils::ECS::EntityRegistry* pRegistry, UUID componentUUID, void* pAddress, const TypeData* pTypeData, YAML::Emitter& out);
		static GLORY_API void DeserializeComponent(EntityScene* pScene, EntitySceneObject* pObject, size_t componentIndex, YAML::Node& object);

	private:
		virtual void Serialize(EntitySceneObject* pObject, YAML::Emitter& out) override;
		virtual Object* Deserialize(Object* pParent, YAML::Node& object, const std::string&, Flags flags) override;
	};
}
