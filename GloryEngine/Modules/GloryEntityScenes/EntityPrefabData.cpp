#include "EntityPrefabData.h"
#include "EntitySceneObject.h"

#include <PropertySerializer.h>

namespace Glory
{
	EntityPrefabData::EntityPrefabData() : m_RootNode(this, nullptr)
	{
		APPEND_TYPE(EntityPrefabData);
	}
	
	EntityPrefabData* EntityPrefabData::CreateFromSceneObject(EntitySceneObject* pSceneObject)
	{
		EntityPrefabData* pPrefab = new EntityPrefabData();
		pPrefab->m_RootNode = pSceneObject;
		return pPrefab;
	}

	const PrefabNode& EntityPrefabData::RootNode() const
	{
		return m_RootNode;
	}

	PrefabNode::PrefabNode(EntityPrefabData* pPrefab, EntitySceneObject* pSceneObject) : m_pPrefab(pPrefab)
	{
		Load(pSceneObject);
	}

	void PrefabNode::operator=(EntitySceneObject* pSceneObject)
	{
		Load(pSceneObject);
	}

	const size_t PrefabNode::ChildCount() const
	{
		return m_Children.size();
	}

	const PrefabNode& PrefabNode::ChildNode(size_t index) const
	{
		return m_Children[index];
	}

	const std::string& PrefabNode::SerializedComponents() const
	{
		return m_SerializedComponents;
	}

	const UUID PrefabNode::OriginalUUID() const
	{
		return m_OriginalUUID;
	}

	const bool PrefabNode::ActiveSelf() const
	{
		return m_ActiveSelf;
	}

	void PrefabNode::Load(EntitySceneObject* pSceneObject)
	{
		if (!pSceneObject) return;
		m_OriginalUUID = pSceneObject->GetUUID();
		m_ActiveSelf = pSceneObject->IsActiveSelf();

		Entity entity = pSceneObject->GetEntityHandle();
		GloryECS::EntityRegistry* pRegistry = entity.GetScene()->GetRegistry();
		EntityID entityID = entity.GetEntityID();
		EntityView* pEntityView = pRegistry->GetEntityView(entityID);
		YAML::Emitter out;
		out << YAML::BeginSeq;
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			out << YAML::BeginMap;
			const uint32_t type = pEntityView->ComponentTypeAt(i);
			const UUID compUUID = pEntityView->ComponentUUIDAt(i);
			out << YAML::Key << "Type";
			out << YAML::Value << type;

			out << YAML::Key << "UUID";
			out << YAML::Value << (uint64_t)compUUID;

			out << YAML::Key << "Properties";
			out << YAML::Value << YAML::BeginMap;
			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(type);
			GloryECS::BaseTypeView* pTypeView = pRegistry->GetTypeView(type);
			for (size_t j = 0; j < pTypeData->FieldCount(); ++j)
			{
				const GloryReflect::FieldData* pFieldData = pTypeData->GetFieldData(j);
				PropertySerializer::SerializeProperty(pFieldData, pTypeView->GetComponentAddress(entityID), out);
			}
			out << YAML::EndMap;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		m_SerializedComponents = out.c_str();

		m_Children.clear();
		for (size_t i = 0; i < pSceneObject->ChildCount(); ++i)
		{
			EntitySceneObject* pChild = (EntitySceneObject*)pSceneObject->GetChild(i);
			m_Children.push_back({ m_pPrefab, pChild });
		}
	}
}
