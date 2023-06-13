#include "EntityPrefabData.h"
#include "EntitySceneObject.h"

#include <PropertySerializer.h>
#include <NodeRef.h>

namespace Glory
{
	EntityPrefabData::EntityPrefabData() : m_RootNode(this, nullptr)
	{
		APPEND_TYPE(EntityPrefabData);
	}

	EntityPrefabData::EntityPrefabData(PrefabNode&& rootNode) noexcept : m_RootNode(std::move(rootNode))
	{
		/* Cache original UUIDs */
		m_OriginalUUIDs.clear();
		m_RootNode.CacheOriginalUUIDs();
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

		/* Cache original UUIDs */
		m_OriginalUUIDs.clear();
		m_RootNode.CacheOriginalUUIDs();
	}

	PrefabNode::PrefabNode(PrefabNode&& other) noexcept
		: m_OriginalUUID(other.m_OriginalUUID), m_ActiveSelf(other.m_ActiveSelf), m_pPrefab(other.m_pPrefab),
		m_Name(std::move(other.m_Name)), m_SerializedComponents(std::move(other.m_SerializedComponents)),
		m_Children(std::move(other.m_Children))
	{
		other.m_OriginalUUID = 0;
		other.m_ActiveSelf = false;
		other.m_pPrefab = nullptr;
	}

	PrefabNode::PrefabNode(EntityPrefabData* pPrefab, EntitySceneObject* pSceneObject) : m_pPrefab(pPrefab)
	{
		Load(pSceneObject);
	}

	PrefabNode::PrefabNode(EntityPrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents)
		: m_OriginalUUID(originalUUID), m_ActiveSelf(activeSelf), m_pPrefab(pPrefab),
		m_Name(name), m_SerializedComponents(serializedComponents), m_Children() {
	}

	void PrefabNode::operator=(EntitySceneObject* pSceneObject)
	{
		Load(pSceneObject);
	}

	void PrefabNode::operator=(PrefabNode&& other) noexcept
	{
		m_OriginalUUID = other.m_OriginalUUID;
		m_ActiveSelf = other.m_ActiveSelf;
		m_pPrefab = other.m_pPrefab;
		m_Name = std::move(other.m_Name);
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

	const std::string& PrefabNode::Name() const
	{
		return m_Name;
	}

	const std::string& PrefabNode::SerializedComponents() const
	{
		return m_SerializedComponents;
	}

	const UUID PrefabNode::OriginalUUID() const
	{
		return m_OriginalUUID;
	}

	const UUID PrefabNode::TransformUUID() const
	{
		return m_TransformUUID;
	}

	const bool PrefabNode::ActiveSelf() const
	{
		return m_ActiveSelf;
	}

	PrefabNode PrefabNode::Create(EntityPrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents)
	{
		return PrefabNode{ pPrefab, originalUUID, transformUUID, activeSelf, name, serializedComponents };
	}

	PrefabNode& PrefabNode::AddChild(EntityPrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents)
	{
		size_t index = m_Children.size();
		m_Children.push_back(PrefabNode{ pPrefab, originalUUID, transformUUID, activeSelf, name, serializedComponents });
		return m_Children[index];
	}

	void PrefabNode::CacheOriginalUUIDs()
	{
		m_pPrefab->m_OriginalUUIDs.push_back(m_OriginalUUID);
		YAML::Node components = YAML::Load(m_SerializedComponents);
		NodeRef componentsRef = components;
		for (size_t i = 0; i < components.size(); ++i)
		{
			NodeValueRef component = componentsRef.ValueRef()[i];
			const UUID uuid = component["UUID"].As<uint64_t>();
			m_pPrefab->m_OriginalUUIDs.push_back(uuid);
		}

		for (size_t i = 0; i < m_Children.size(); ++i)
		{
			m_Children[i].CacheOriginalUUIDs();
		}
	}

	void PrefabNode::Load(EntitySceneObject* pSceneObject)
	{
		if (!pSceneObject) return;
		m_OriginalUUID = pSceneObject->GetUUID();
		m_ActiveSelf = pSceneObject->IsActiveSelf();
		m_Name = pSceneObject->Name();
		m_pPrefab->m_OriginalUUIDs.push_back(m_OriginalUUID);

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
			if (i == 0) m_TransformUUID = compUUID;

			m_pPrefab->m_OriginalUUIDs.push_back(compUUID);
			out << YAML::Key << "TypeHash";
			out << YAML::Value << type;

			out << YAML::Key << "UUID";
			out << YAML::Value << (uint64_t)compUUID;

			out << YAML::Key << "Properties";
			out << YAML::Value << YAML::BeginMap;
			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(type);
			GloryECS::BaseTypeView* pTypeView = pRegistry->GetTypeView(type);
			void* compAddress = pTypeView->GetComponentAddress(entityID);
			for (size_t j = 0; j < pTypeData->FieldCount(); ++j)
			{
				const GloryReflect::FieldData* pFieldData = pTypeData->GetFieldData(j);
				void* pAddress = pFieldData->GetAddress(compAddress);
				PropertySerializer::SerializeProperty(pFieldData, pAddress, out);
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
