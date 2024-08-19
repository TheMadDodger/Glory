#include "PrefabData.h"
#include "GScene.h"
#include "PropertySerializer.h"
#include "GScene.h"
#include "SceneManager.h"
#include "Engine.h"

#include <yaml-cpp/yaml.h>
#include <NodeRef.h>
#include <TypeData.h>

namespace Glory
{
	/* @fixme Remove this when prefabs no longer require yaml data */
	/*void SerializeComponent(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityView* pEntityView, Utils::ECS::EntityID entity, size_t index, Utils::NodeValueRef node)
	{
		out << YAML::BeginMap;
		const UUID compUUID = pEntityView->ComponentUUIDAt(index);
		out << YAML::Key << "UUID";
		out << YAML::Value << uint64_t(compUUID);

		const uint32_t type = pEntityView->ComponentTypeAt(index);
		const Utils::Reflect::TypeData* pType = Utils::Reflect::Reflect::GetTyeData(type);

		out << YAML::Key << "TypeName";
		out << YAML::Value << pType->TypeName();

		out << YAML::Key << "TypeHash";
		out << YAML::Value << uint64_t(type);

		out << YAML::Key << "Active";
		out << YAML::Value << pRegistry->GetTypeView(type)->IsActive(entity);

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		pEngine->GetSerializers().SerializeProperty("Properties", pType, pRegistry->GetComponentAddress(entity, compUUID), node);
		out << YAML::EndMap;
	}*/

	PrefabData::PrefabData()
	{
		APPEND_TYPE(PrefabData);
	}

	PrefabData* PrefabData::CreateFromEntity(GScene* pScene, Utils::ECS::EntityID entity)
	{
		PrefabData* pPrefab = new PrefabData();
		pPrefab->SetName(pScene->EntityName(entity));
		CopyEntity(pPrefab, pScene, entity, 0);
		return pPrefab;
	}

	void PrefabData::CopyEntity(PrefabData* pPrefab, GScene* pScene, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent)
	{
		const UUID uuid = pScene->GetEntityUUID(entity);
		Utils::ECS::EntityRegistry& registry = pScene->GetRegistry();
		const Utils::ECS::EntityID newEntity = registry.CopyEntityToOtherRegistry(entity, parent, &pPrefab->m_Registry);
		pPrefab->m_UUIds.emplace(newEntity, uuid);
		pPrefab->m_Ids.emplace(uuid, newEntity);
		pPrefab->m_Names.emplace(newEntity, pScene->EntityName(entity));
		Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
		{
			const Utils::ECS::EntityID child = pEntityView->Child(i);
			CopyEntity(pPrefab, pScene, child, newEntity);
		}
	}

	//const PrefabNode& PrefabData::RootNode() const
	//{
	//	return m_RootNode;
	//}

	//void PrefabData::SetRootNode(PrefabNode&& node)
	//{
	//	m_RootNode = std::move(node);

	//	/* Cache original UUIDs */
	//	m_OriginalUUIDs.clear();
	//	m_RootNode.CacheOriginalUUIDs();
	//}

	//PrefabNode::PrefabNode(PrefabNode&& other) noexcept
	//	: m_OriginalUUID(other.m_OriginalUUID), m_ActiveSelf(other.m_ActiveSelf), m_pPrefab(other.m_pPrefab),
	//	m_Name(std::move(other.m_Name)), m_SerializedComponents(std::move(other.m_SerializedComponents)),
	//	m_Children(std::move(other.m_Children))
	//{
	//	other.m_OriginalUUID = 0;
	//	other.m_ActiveSelf = false;
	//	other.m_pPrefab = nullptr;
	//}

	//PrefabNode::PrefabNode(PrefabData* pPrefab, const Entity& entity) : m_pPrefab(pPrefab)
	//{
	//	Load(entity);
	//}

	//PrefabNode::PrefabNode(PrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents)
	//	: m_OriginalUUID(originalUUID), m_ActiveSelf(activeSelf), m_pPrefab(pPrefab),
	//	m_Name(name), m_SerializedComponents(serializedComponents), m_Children() {
	//}

	//void PrefabNode::operator=(const Entity& entity)
	//{
	//	Load(entity);
	//}

	//void PrefabNode::operator=(PrefabNode&& other) noexcept
	//{
	//	m_OriginalUUID = other.m_OriginalUUID;
	//	m_ActiveSelf = other.m_ActiveSelf;
	//	m_pPrefab = other.m_pPrefab;
	//	m_Name = std::move(other.m_Name);
	//	m_SerializedComponents = std::move(other.m_SerializedComponents);
	//	m_Children = std::move(other.m_Children);

	//	other.m_OriginalUUID = 0;
	//	other.m_ActiveSelf = false;
	//	other.m_pPrefab = nullptr;
	//}

	//const size_t PrefabNode::ChildCount() const
	//{
	//	return m_Children.size();
	//}

	//const PrefabNode& PrefabNode::ChildNode(size_t index) const
	//{
	//	return m_Children[index];
	//}

	//const std::string& PrefabNode::Name() const
	//{
	//	return m_Name;
	//}

	//const std::string& PrefabNode::SerializedComponents() const
	//{
	//	return m_SerializedComponents;
	//}

	//const UUID PrefabNode::OriginalUUID() const
	//{
	//	return m_OriginalUUID;
	//}

	//const UUID PrefabNode::TransformUUID() const
	//{
	//	return m_TransformUUID;
	//}

	//const bool PrefabNode::ActiveSelf() const
	//{
	//	return m_ActiveSelf;
	//}

	//PrefabNode PrefabNode::Create(PrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents)
	//{
	//	return PrefabNode{ pPrefab, originalUUID, transformUUID, activeSelf, name, serializedComponents };
	//}

	//PrefabNode& PrefabNode::AddChild(PrefabData* pPrefab, UUID originalUUID, UUID transformUUID, bool activeSelf, const std::string& name, const std::string& serializedComponents)
	//{
	//	size_t index = m_Children.size();
	//	m_Children.push_back(PrefabNode{ pPrefab, originalUUID, transformUUID, activeSelf, name, serializedComponents });
	//	return m_Children[index];
	//}

	//void PrefabNode::CacheOriginalUUIDs()
	//{
	//	m_pPrefab->m_OriginalUUIDs.push_back(m_OriginalUUID);
	//	YAML::Node components = YAML::Load(m_SerializedComponents);
	//	Utils::NodeRef componentsRef = components;
	//	for (size_t i = 0; i < components.size(); ++i)
	//	{
	//		Utils::NodeValueRef component = componentsRef.ValueRef()[i];
	//		const UUID uuid = component["UUID"].As<uint64_t>();
	//		m_pPrefab->m_OriginalUUIDs.push_back(uuid);
	//	}

	//	for (size_t i = 0; i < m_Children.size(); ++i)
	//	{
	//		m_Children[i].CacheOriginalUUIDs();
	//	}
	//}

	//void PrefabNode::Load(const Entity& entity)
	//{
	//	if (!entity.IsValid()) return;
	//	Utils::ECS::EntityID entityID = entity.GetEntityID();

	//	m_OriginalUUID = entity.GetScene()->GetEntityUUID(entityID);
	//	m_ActiveSelf = entity.IsActiveSelf();
	//	m_Name = entity.GetScene()->EntityName(entityID);
	//	m_pPrefab->m_OriginalUUIDs.push_back(m_OriginalUUID);

	//	Utils::ECS::EntityRegistry* pRegistry = entity.GetRegistry();
	//	Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entityID);
	//	YAML::Emitter out;
	//	out << YAML::BeginSeq;
	//	for (size_t i = 0; i < pEntityView->ComponentCount(); ++i)
	//	{
	//		const UUID compUUID = pEntityView->ComponentUUIDAt(i);
	//		if (i == 0) m_TransformUUID = compUUID;
	//		m_pPrefab->m_OriginalUUIDs.push_back(compUUID);
	//		SerializeComponent(pRegistry, pEntityView, entityID, i, out);
	//	}
	//	out << YAML::EndSeq;
	//	m_SerializedComponents = out.c_str();

	//	m_Children.clear();
	//	for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
	//	{
	//		Utils::ECS::EntityID childEntity = pEntityView->Child(i);
	//		m_Children.push_back({ m_pPrefab, { childEntity, entity.GetScene() } });
	//	}
	//}
}
