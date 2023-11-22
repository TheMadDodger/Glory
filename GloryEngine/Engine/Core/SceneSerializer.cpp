#include "SceneSerializer.h"
#include "PropertySerializer.h"

#include "Components.h"

#include <Reflection.h>

namespace Glory
{
	SceneSerializer::SceneSerializer()
	{
	}

	SceneSerializer::~SceneSerializer()
	{
	}

	void SceneSerializer::SerializeComponent(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityView* pEntityView, Utils::ECS::EntityID entity, size_t index, YAML::Emitter& out)
	{
		const UUID compUUID = pEntityView->ComponentUUIDAt(index);
		out << YAML::Key << "UUID";
		out << YAML::Value << uint64_t(compUUID);

		const uint32_t type = pEntityView->ComponentTypeAt(index);
		const Utils::Reflect::TypeData* pType = Utils::Reflect::Reflect::GetTyeData(type);

		out << YAML::Key << "TypeName";
		out << YAML::Value << pType->TypeName();

		out << YAML::Key << "TypeHash";
		out << YAML::Value << uint64_t(type);

		PropertySerializer::SerializeProperty("Properties", pType, pRegistry->GetComponentAddress(entity, compUUID), out);
	}

	void SceneSerializer::DeserializeComponent()
	{
	}

	void SceneSerializer::Serialize(GScene* pScene, YAML::Emitter& out)
	{
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		pScene->GetRegistry().ForEach([&](Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity) {
			Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
			const UUID uuid = pScene->GetEntityUUID(entity);
			
			out << YAML::Key << "Name";
			out << YAML::Value << "";

			out << YAML::Key << "UUID";
			out << YAML::Value << uint64_t(uuid);

			out << YAML::Key << "ParentUUID";

			Entity parent = pScene->GetEntity(pEntityView->Parent());
			out << YAML::Value << (parent.IsValid() ?
				uint64_t(pScene->GetEntityUUID(parent.GetEntityID())) : 0);

			out << YAML::Key << "SiblingIndex";
			out << YAML::Value << 0;

			out << YAML::Key << "Components";
			out << YAML::Value << YAML::BeginSeq;
			for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
			{
				SerializeComponent(pRegistry, pEntityView, entity, i, out);
			}
			out << YAML::EndSeq;
		});
		out << YAML::EndSeq;

	}

	Object* SceneSerializer::Deserialize(Object* pParent, YAML::Node& object, const std::string& name, Flags flags)
	{
		return nullptr;
	}
}
