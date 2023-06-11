#include "EntityPrefabData.h"
#include "EntitySceneObject.h"

#include <PropertySerializer.h>

namespace Glory
{
	EntityPrefabData::EntityPrefabData() : m_RootNode(this, nullptr)
	{
		APPEND_TYPE(EntityPrefabData);
	}

	EntityPrefabData::EntityPrefabData(PrefabNode&& rootNode) noexcept : m_RootNode(std::move(rootNode))
	{
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

	void EntityPrefabData::SetRootNode(PrefabNode&& node)
	{
		m_RootNode = std::move(node);
	}

	PrefabNode::PrefabNode(PrefabNode&& other) noexcept
		: m_OriginalUUID(other.m_OriginalUUID), m_ActiveSelf(other.m_ActiveSelf), m_pPrefab(other.m_pPrefab),
		m_SerializedComponents(std::move(other.m_SerializedComponents)), m_Children(std::move(other.m_Children))
	{
		other.m_OriginalUUID = 0;
		other.m_ActiveSelf = false;
		other.m_pPrefab = nullptr;
	}

	PrefabNode::PrefabNode(EntityPrefabData* pPrefab, EntitySceneObject* pSceneObject) : m_pPrefab(pPrefab)
	{
		Load(pSceneObject);
	}

	PrefabNode::PrefabNode(EntityPrefabData* pPrefab, UUID originalUUID, bool activeSelf, const std::string& serializedComponents)
		: m_OriginalUUID(originalUUID), m_ActiveSelf(activeSelf), m_pPrefab(pPrefab),
		m_SerializedComponents(serializedComponents), m_Children() {}

	void PrefabNode::operator=(EntitySceneObject* pSceneObject)
	{
		Load(pSceneObject);
	}

	void PrefabNode::operator=(PrefabNode&& other)
	{
		m_OriginalUUID = other.m_OriginalUUID;
		m_ActiveSelf = other.m_ActiveSelf;
		m_pPrefab = other.m_pPrefab;
		m_SerializedComponents = std::move(other.m_SerializedComponents);
		m_Children = std::move(other.m_Children);

		other.m_OriginalUUID = 0;
		other.m_ActiveSelf = false;
		other.m_pPrefab = nullptr;
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

	PrefabNode PrefabNode::Create(EntityPrefabData* pPrefab, UUID originalUUID, bool activeSelf, const std::string& serializedComponents)
	{
		return PrefabNode{ pPrefab, originalUUID, activeSelf, serializedComponents };
	}

	PrefabNode& PrefabNode::AddChild(EntityPrefabData* pPrefab, UUID originalUUID, bool activeSelf, const std::string& serializedComponents)
	{
		size_t index = m_Children.size();
		m_Children.push_back(PrefabNode{ pPrefab, originalUUID, activeSelf, serializedComponents });
		return m_Children[index];
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
