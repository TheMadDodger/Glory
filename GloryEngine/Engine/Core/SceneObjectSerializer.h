#pragma once
#include "Serializer.h"
#include "SceneObject.h"

#include <Reflection.h>

namespace Glory
{
	class SceneObjectSerializer : public SerializerTemplate<SceneObject>
	{
	public:
		SceneObjectSerializer();
		virtual ~SceneObjectSerializer();

		static GLORY_API void SerializeComponent(Glory::Utils::ECS::EntityID entityID, Glory::Utils::ECS::EntityRegistry* pRegistry, UUID componentUUID, void* pAddress, const TypeData* pTypeData, YAML::Emitter& out);
		static GLORY_API void DeserializeComponent(GScene* pScene, SceneObject* pObject, size_t componentIndex, YAML::Node& object);

	private:
		virtual void Serialize(SceneObject* pObject, YAML::Emitter& out) override;
		virtual Object* Deserialize(Object* pParent, YAML::Node& object, const std::string&, Flags flags) override;
	};
}
