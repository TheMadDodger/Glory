#include "PrefabData.h"
#include "SceneObject.h"
#include "SceneObjectSerializer.h"

#include <yaml-cpp/yaml.h>
#include <NodeRef.h>

namespace Glory
{
	PrefabData::PrefabData() : m_RootNode(this, nullptr)
	{
		APPEND_TYPE(PrefabData);
	}

	PrefabData::PrefabData(PrefabNode&& rootNode) noexcept : m_RootNode(std::move(rootNode))
	{
		/* Cache original UUIDs */
		m_OriginalUUIDs.clear();
		m_RootNode.CacheOriginalUUIDs();
	}

	PrefabData* PrefabData::CreateFromSceneObject(SceneObject* pSceneObject)
	{
		PrefabData* pPrefab = new PrefabData();
		pPrefab->SetName(pSceneObject->Name());
		pPrefab->m_RootNode = pSceneObject;
		return pPrefab;
	}

	const PrefabNode& PrefabData::RootNode() const
	{
		return m_RootNode;
	}

	void PrefabData::SetRootNode(PrefabNode&& node)
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

	PrefabNode::PrefabNode(PrefabData* pPrefab, SceneObject* pSceneObject) : m_pPrefab(pPrefab)
	{
		Load(pSceneObject);
	}

	PrefabNode::PrefabNode(PrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents)
		: m_OriginalUUID(originalUUID), m_ActiveSelf(activeSelf), m_pPrefab(pPrefab),
		m_Name(name), m_SerializedComponents(serializedComponents), m_Children() {
	}

	void PrefabNode::operator=(SceneObject* pSceneObject)
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

	PrefabNode PrefabNode::Create(PrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents)
	{
		return PrefabNode{ pPrefab, originalUUID, transformUUID, activeSelf, name, serializedComponents };
	}

	PrefabNode& PrefabNode::AddChild(PrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents)
	{
		size_t index = m_Children.size();
		m_Children.push_back(PrefabNode{ pPrefab, originalUUID, transformUUID, activeSelf, name, serializedComponents });
		return m_Children[index];
	}

	void PrefabNode::CacheOriginalUUIDs()
	{
		m_pPrefab->m_OriginalUUIDs.push_back(m_OriginalUUID);
		YAML::Node components = YAML::Load(m_SerializedComponents);
		Utils::NodeRef componentsRef = components;
		for (size_t i = 0; i < components.size(); ++i)
		{
			Utils::NodeValueRef component = componentsRef.ValueRef()[i];
			const UUID uuid = component["UUID"].As<uint64_t>();
			m_pPrefab->m_OriginalUUIDs.push_back(uuid);
		}

		for (size_t i = 0; i < m_Children.size(); ++i)
		{
			m_Children[i].CacheOriginalUUIDs();
		}
	}

	void PrefabNode::Load(SceneObject* pSceneObject)
	{
		if (!pSceneObject) return;
		m_OriginalUUID = pSceneObject->GetUUID();
		m_ActiveSelf = pSceneObject->IsActiveSelf();
		m_Name = pSceneObject->Name();
		m_pPrefab->m_OriginalUUIDs.push_back(m_OriginalUUID);

		Entity entity = pSceneObject->GetEntityHandle();
		Utils::ECS::EntityRegistry* pRegistry = entity.GetRegistry();
		Utils::ECS::EntityID entityID = entity.GetEntityID();
		Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entityID);
		YAML::Emitter out;
		out << YAML::BeginSeq;
		for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
		{
			const uint32_t type = pEntityView->ComponentTypeAt(i);
			const UUID compUUID = pEntityView->ComponentUUIDAt(i);
			if (i == 0) m_TransformUUID = compUUID;
			m_pPrefab->m_OriginalUUIDs.push_back(compUUID);

			const Utils::Reflect::TypeData* pTypeData = Utils::Reflect::Reflect::GetTyeData(type);
			Utils::ECS::BaseTypeView* pTypeView = pRegistry->GetTypeView(type);
			void* compAddress = pTypeView->GetComponentAddress(entityID);
			SceneObjectSerializer::SerializeComponent(entityID, pRegistry, compUUID, compAddress, pTypeData, out);
		}
		out << YAML::EndSeq;
		m_SerializedComponents = out.c_str();

		m_Children.clear();
		for (size_t i = 0; i < pSceneObject->ChildCount(); ++i)
		{
			SceneObject* pChild = pSceneObject->GetChild(i);
			m_Children.push_back({ m_pPrefab, pChild });
		}
	}
}
