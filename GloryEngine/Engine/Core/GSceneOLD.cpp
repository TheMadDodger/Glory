//#include "GScene.h"
//#include "Debug.h"
//#include "Components.h"
//#include "PrefabData.h"
//#include "DistributedRandom.h"
//#include "SceneObject.h"
//#include "PropertySerializer.h"
//
//#include <algorithm>
//#include <NodeRef.h>
//
//namespace Glory
//{
//	SceneObject* GScene::GetSceneObject(size_t index)
//	{
//		if (index >= m_pSceneObjects.size()) return nullptr;
//		return m_pSceneObjects[index];
//	}
//
//	void GScene::DeleteObject(SceneObject* pObject)
//	{
//		OnDeleteObject(pObject);
//		auto it = std::find(m_pSceneObjects.begin(), m_pSceneObjects.end(), pObject);
//		if (it == m_pSceneObjects.end())
//		{
//			Debug::LogError("Can't delete object from scene that does not own it!");
//			return;
//		}
//		m_pSceneObjects.erase(it);
//		pObject->DestroyOwnChildren();
//		pObject->SetParent(nullptr);
//		delete pObject;
//	}
//
//	SceneObject* GScene::FindSceneObject(UUID uuid) const
//	{
//		auto it = std::find_if(m_pSceneObjects.begin(), m_pSceneObjects.end(), [&](SceneObject* pObject) { return pObject->GetUUID() == uuid; });
//		if (it == m_pSceneObjects.end()) return nullptr;
//		return *it;
//	}
//
//	void GScene::DelayedSetParent(SceneObject* pObjectToParent, UUID parentID)
//	{
//		if (pObjectToParent == nullptr || parentID == NULL) return;
//		m_DelayedParents.emplace_back(DelayedParentData(pObjectToParent, parentID));
//	}
//
//	void GScene::HandleDelayedParents()
//	{
//		std::for_each(m_DelayedParents.begin(), m_DelayedParents.end(), [&](const DelayedParentData& data) { OnDelayedSetParent(data); });
//		m_DelayedParents.clear();
//	}
//
//	void GScene::UnsetPrefab(SceneObject* pObject)
//	{
//		m_ActivePrefabs.erase(pObject->GetUUID());
//
//		for (size_t i = 0; i < pObject->m_pChildren.size(); ++i)
//		{
//			SceneObject* pChild = pObject->m_pChildren[i];
//			UnsetChildrenPrefab(pChild);
//		}
//	}
//
//	const UUID GScene::Prefab(UUID objectID) const
//	{
//		const auto itor = m_ActivePrefabs.find(objectID);
//		return itor != m_ActivePrefabs.end() ? itor->second : 0;
//	}
//
//	const UUID GScene::PrefabChild(UUID objectID) const
//	{
//		const auto itor = m_ActivePrefabChildren.find(objectID);
//		return itor != m_ActivePrefabChildren.end() ? itor->second : 0;
//	}
//
//	SceneObject* GScene::InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab, const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
//	{
//		UUID uuid = UUID();
//		const uint32_t first32Bits = uint32_t((uuid << 32) >> 32);
//		const uint32_t second32Bits = uint32_t(uuid >> 32);
//		const uint32_t seed = first32Bits & second32Bits;
//		UUIDRemapper remapper{ seed };
//		remapper.EnforceRemap(pPrefab->RootNode().OriginalUUID(), uuid);
//		return InstantiatePrefab(pParent, pPrefab, RandomDevice::Seed(), pos, rot, scale);
//	}
//
//	SceneObject* GScene::InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab, uint32_t remapSeed,
//		const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
//	{
//		UUIDRemapper remapper{ remapSeed };
//		const PrefabNode& rootNode = pPrefab->RootNode();
//		SceneObject* pInstantiatedPrefab = InstantiatePrefabNode(pParent, rootNode, remapper);
//
//		Transform& transform = pInstantiatedPrefab->GetEntityHandle().GetComponent<Transform>();
//		transform.Position = pos;
//		transform.Rotation = rot;
//		transform.Scale = scale;
//
//		SetPrefab(pInstantiatedPrefab, pPrefab->GetUUID());
//		return pInstantiatedPrefab;
//	}
//
//	SceneObject* GScene::InstantiatePrefab(SceneObject* pParent, PrefabData* pPrefab, UUIDRemapper& remapper,
//		const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale)
//	{
//		const PrefabNode& rootNode = pPrefab->RootNode();
//		SceneObject* pInstantiatedPrefab = InstantiatePrefabNode(pParent, rootNode, remapper);
//
//		Transform& transform = pInstantiatedPrefab->GetEntityHandle().GetComponent<Transform>();
//		transform.Position = pos;
//		transform.Rotation = rot;
//		transform.Scale = scale;
//
//		SetPrefab(pInstantiatedPrefab, pPrefab->GetUUID());
//		return pInstantiatedPrefab;
//	}
//
//	SceneObject* GScene::GetSceneObjectFromEntityID(Utils::ECS::EntityID entity)
//	{
//		if (m_EntityIDToObject.find(entity) == m_EntityIDToObject.end()) return nullptr;
//		return m_EntityIDToObject[entity];
//	}
//
//	void GScene::OnDelayedSetParent(const DelayedParentData& data)
//	{
//		SceneObject* pParent = FindSceneObject(data.ParentID);
//		if (pParent == nullptr)
//		{
//			Debug::LogError("Could not set delayed parent for object " + data.ObjectToParent->Name() + " because the parent does not exist!");
//			return;
//		}
//		data.ObjectToParent->SetParent(pParent);
//	}
//
//	void GScene::SetChildrenPrefab(SceneObject* pObject, UUID prefabID)
//	{
//		m_ActivePrefabChildren.emplace(pObject->GetUUID(), prefabID);
//
//		for (size_t i = 0; i < pObject->m_pChildren.size(); ++i)
//		{
//			SceneObject* pChild = pObject->m_pChildren[i];
//			SetChildrenPrefab(pChild, prefabID);
//		}
//	}
//
//	void GScene::UnsetChildrenPrefab(SceneObject* pObject)
//	{
//		m_ActivePrefabChildren.erase(pObject->GetUUID());
//
//		for (size_t i = 0; i < pObject->m_pChildren.size(); ++i)
//		{
//			SceneObject* pChild = pObject->m_pChildren[i];
//			UnsetChildrenPrefab(pChild);
//		}
//	}
//
//	Entity GScene::CreateEntity(UUID uuid, UUID transUUID)
//	{
//		Utils::ECS::EntityID entityID = m_Registry.CreateEntity<Transform>(transUUID);
//		return Entity(entityID, this);
//	}
//
//	SceneObject* GScene::InstantiatePrefabNode(SceneObject* pParent, const PrefabNode& node, UUIDRemapper& remapper)
//	{
//		const UUID objectID = remapper(node.OriginalUUID());
//		const UUID transformID = remapper(node.TransformUUID());
//		SceneObject* pObject = CreateEmptyObject(node.Name(), objectID, transformID);
//		if (pParent)
//			pObject->SetParent(pParent);
//
//		pObject->SetActive(node.ActiveSelf());
//
//		const std::string& serializedComponents = node.SerializedComponents();
//		YAML::Node components = YAML::Load(serializedComponents);
//
//		const uint32_t transformTypeHash = ResourceType::GetHash(typeid(Transform));
//		const uint32_t scriptedTypeHash = ResourceType::GetHash(typeid(ScriptedComponent));
//
//		size_t currentComponentIndex = 0;
//		for (size_t i = 0; i < components.size(); ++i)
//		{
//			YAML::Node nextObject = components[i];
//			YAML::Node subNode;
//			uint32_t typeHash = 0;
//			UUID originalUUID = 0;
//			std::string typeName = "";
//			YAML_READ(nextObject, subNode, TypeHash, typeHash, uint32_t);
//			YAML_READ(nextObject, subNode, UUID, originalUUID, uint64_t);
//
//			Entity entityHandle = pObject->GetEntityHandle();
//			Utils::ECS::EntityID entity = entityHandle.GetEntityID();
//			Utils::ECS::EntityRegistry* pRegistry = GetRegistry();
//
//			UUID compUUID = remapper(originalUUID);
//
//			void* pComponentAddress = nullptr;
//			if (typeHash != transformTypeHash) pComponentAddress = pRegistry->CreateComponent(entity, typeHash, compUUID);
//			else
//			{
//				Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
//				compUUID = pEntityView->ComponentUUIDAt(0);
//				pComponentAddress = pRegistry->GetComponentAddress(entity, compUUID);
//			}
//
//			const Utils::Reflect::TypeData* pTypeData = Utils::Reflect::Reflect::GetTyeData(typeHash);
//			YAML::Node originalProperties = nextObject["Properties"];
//			if (typeHash != scriptedTypeHash)
//			{
//				PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, originalProperties);
//			}
//			else
//			{
//				YAML::Node finalProperties = YAML::Node(YAML::NodeType::Map);
//
//				Utils::NodeRef originalPropertiesRef = originalProperties;
//				Utils::NodeRef finalPropertiesRef = finalProperties;
//
//				Utils::NodeValueRef props = originalPropertiesRef.ValueRef();
//				Utils::NodeValueRef finalProps = finalPropertiesRef.ValueRef();
//
//				finalPropertiesRef["m_Script"].Set(originalPropertiesRef["m_Script"].As<uint64_t>());
//				YAML::Node scriptData = originalProperties["ScriptData"];
//				for (YAML::const_iterator itor = scriptData.begin(); itor != scriptData.end(); ++itor)
//				{
//					const std::string name = itor->first.as<std::string>();
//					Utils::NodeValueRef prop = props["ScriptData"][name];
//					if (!prop.IsMap())
//					{
//						finalProps[name].Set(prop.Node());
//						continue;
//					}
//
//					Utils::NodeValueRef originalSceneUUD = prop["SceneUUID"];
//					Utils::NodeValueRef originalObjectUUD = prop["ObjectUUID"];
//
//					Utils::NodeValueRef sceneUUID = finalProps["ScriptData"][name]["SceneUUID"];
//					Utils::NodeValueRef objectUUID = finalProps["ScriptData"][name]["ObjectUUID"];
//
//					if (!originalSceneUUD.Exists() || !originalObjectUUD.Exists())
//					{
//						finalProps["ScriptData"][name].Set(prop.Node());
//						continue;
//					}
//
//					sceneUUID.Set((uint64_t)GetUUID());
//					const UUID uuid = originalObjectUUD.As<uint64_t>();
//					UUID remapped;
//					if (!remapper.Find(uuid, remapped))
//						remapped = uuid;
//
//					objectUUID.Set(remapped);
//				}
//
//				PropertySerializer::DeserializeProperty(pTypeData, pComponentAddress, finalProperties);
//			}
//
//			pRegistry->GetTypeView(typeHash)->Invoke(Utils::ECS::InvocationType::OnValidate, pRegistry, entity, pComponentAddress);
//			++currentComponentIndex;
//		}
//
//		for (size_t i = 0; i < node.ChildCount(); ++i)
//		{
//			const PrefabNode& childNode = node.ChildNode(i);
//			InstantiatePrefabNode(pObject, childNode, remapper);
//		}
//
//		return pObject;
//	}
//}