#include "MonoScriptedManager.h"
#include "MonoComponents.h"
#include "MonoManager.h"
#include "CoreLibManager.h"

#include <GScene.h>
#include <IEngine.h>
#include <SceneManager.h>
#include <Debug.h>
#include <UUIDRemapper.h>
#include <SceneObjectRef.h>

namespace Glory
{
	MonoScriptedManager::MonoScriptedManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity)
	{
	}

	MonoScriptedManager::~MonoScriptedManager()
	{
	}

	void MonoScriptedManager::OnSerialize(Utils::BinaryStream& stream) const
	{
	}

	void MonoScriptedManager::OnDeserialize(Utils::BinaryStream& stream)
	{
	}

	void MonoScriptedManager::OnCopy(MonoScriptComponent& script)
	{
	}

	void MonoScriptedManager::UnpackDataInto(const void* data, MonoScriptComponent& newComponent)
	{
		const MonoScriptComponent* pScriptSource = reinterpret_cast<const MonoScriptComponent*>(data);
		newComponent.m_ScriptType = pScriptSource->m_ScriptType;
		newComponent.m_ScriptData.m_Buffer.resize(pScriptSource->m_ScriptData.m_Buffer.size());
		std::memcpy(newComponent.m_ScriptData.m_Buffer.data(), pScriptSource->m_ScriptData.m_Buffer.data(),
			pScriptSource->m_ScriptData.m_Buffer.size());
	}

	void MonoScriptedManager::SerializeDense(Utils::BinaryStream& stream) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const MonoScriptComponent& component = GetAt(i);
			stream.Write<char>(component.m_ScriptData.m_Buffer).Write(component.m_ScriptType);
		}
	}

	void MonoScriptedManager::DeserializeDense(Utils::BinaryStream& stream)
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			MonoScriptComponent& component = GetAt(i);
			stream.Read<char>(component.m_ScriptData.m_Buffer).Read(component.m_ScriptType);
		}
	}

	void MonoScriptedManager::OnStartImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{

		int typeIndex = m_pScriptManager->TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		static const uint32_t scriptTypeHash = MonoScriptComponent::GetTypeData()->TypeHash();
		for (size_t i = 0; i < m_pRegistry->EntityComponentCount(entity); ++i)
		{
			const uint32_t typeHash = m_pRegistry->EntityComponentType(entity, i);

			if (typeHash != scriptTypeHash) continue;
			pComponent.m_CachedComponentID = m_pRegistry->EntityComponentID(entity, i);
			break;
		}

		GScene* pScene = m_pRegistry->GetUserData<GScene>();
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		MonoObject* pScriptObject = m_pCoreLibManager->GetScript(sceneID, entityUuid, pComponent.m_CachedComponentID);
		pComponent.m_ScriptObjectHandle = mono_gchandle_new_weakref(pScriptObject, false);

		m_pScriptManager->ReadDefaults((size_t)typeIndex, pComponent.m_ScriptData.m_Buffer);
		m_pScriptManager->SetPropertyValues((size_t)typeIndex, pScriptObject, pComponent.m_ScriptData.m_Buffer);
		m_pScriptManager->Invoke((size_t)typeIndex, pScriptObject, "Start", nullptr);
	}

	void MonoScriptedManager::OnStopImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		int typeIndex = m_pScriptManager->TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		m_pScriptManager->Invoke((size_t)typeIndex, pScriptObject, "Stop", nullptr);
	}

	void MonoScriptedManager::OnValidateImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		static const uint32_t scriptTypeHash = MonoScriptComponent::GetTypeData()->TypeHash();
		for (size_t i = 0; i < m_pRegistry->EntityComponentCount(entity); ++i)
		{
			const uint32_t typeHash = m_pRegistry->EntityComponentType(entity, i);

			if (typeHash != scriptTypeHash) continue;
			pComponent.m_CachedComponentID = m_pRegistry->EntityComponentID(entity, i);
			break;
		}

		int typeIndex = m_pScriptManager->TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		GScene* pScene = m_pRegistry->GetUserData<GScene>();
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		MonoObject* pScriptObject = m_pCoreLibManager->CreateScript((size_t)typeIndex, sceneID, entityUuid, pComponent.m_CachedComponentID);
		pComponent.m_ScriptObjectHandle = mono_gchandle_new_weakref(pScriptObject, false);
		m_pScriptManager->ReadDefaults((size_t)typeIndex, pComponent.m_ScriptData.m_Buffer);
		m_pScriptManager->SetPropertyValues((size_t)typeIndex, pScriptObject, pComponent.m_ScriptData.m_Buffer);
		m_pScriptManager->Invoke((size_t)typeIndex, pScriptObject, "OnValidate", nullptr);
	}

	void MonoScriptedManager::OnEnableImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		int typeIndex = m_pScriptManager->TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		m_pScriptManager->Invoke((size_t)typeIndex, pScriptObject, "OnEnable", nullptr);
	}

	void MonoScriptedManager::OnDisableImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		int typeIndex = m_pScriptManager->TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		m_pScriptManager->Invoke((size_t)typeIndex, pScriptObject, "OnDisable", nullptr);
	}

	void MonoScriptedManager::OnUpdateImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent, float)
	{
		int typeIndex = m_pScriptManager->TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		GScene* pScene = m_pRegistry->GetUserData<GScene>();
		const UUID entityUuid = pScene->GetEntityUUID(entity);
		const UUID sceneID = pScene->GetUUID();
		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		m_pScriptManager->Invoke((size_t)typeIndex, pScriptObject, "Update", nullptr);

		/** @todo: This should be handled by the editor somehow */
		//if (pComponent.m_ScriptData.m_Buffer.empty()) pComponent.m_ScriptData.m_Buffer = std::vector<char>(4, '\0');
		//scriptManager.GetPropertyValues((size_t)typeIndex, pScriptObject, pComponent.m_ScriptData.m_Buffer);
	}

	void MonoScriptedManager::OnDrawImpl(Utils::ECS::EntityID entity, MonoScriptComponent& pComponent)
	{
		int typeIndex = m_pScriptManager->TypeIndexFromHash(pComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		MonoObject* pScriptObject = mono_gchandle_get_target(pComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		m_pScriptManager->Invoke((size_t)typeIndex, pScriptObject, "Draw", nullptr);
	}

	void MonoScriptedManager::OnCopy(GScene* pScene, void* data, UUID componentId, UUIDRemapper& remapper)
	{
		MonoScriptComponent& component = *static_cast<MonoScriptComponent*>(data);
		component.m_ScriptObjectHandle = 0;
		component.m_CachedComponentID = 0;
		if (!component.m_ScriptType.m_Hash) return;
		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		const int typeIndex = scriptManager.TypeIndexFromHash(component.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;
		component.m_CachedComponentID = componentId;
		auto& properties = scriptManager.ScriptProperties(typeIndex);
		for (size_t i = 0; i < properties.size(); ++i)
		{
			auto& prop = properties[i];
			if (prop.m_TypeHash != ST_Object) continue;
			char* idData = &component.m_ScriptData.m_Buffer[prop.m_RelativeOffset];
			SceneObjectRef* id = reinterpret_cast<SceneObjectRef*>(idData);
			const UUID newId = remapper(id->ObjectUUID());
			*id->ObjectUUIDMember() = newId;
			*id->SceneUUIDMember() = pScene->GetUUID();
		}
	}

	void MonoScriptedManager::GetReferencesImpl(std::vector<UUID>& references) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const MonoScriptComponent& scriptComponent = GetAt(i);
			if (!scriptComponent.m_ScriptType.m_Hash) continue;
			MonoScriptManager& scripts = MonoManager::Instance()->GetCoreLibManager()->ScriptManager();
			const int typeIndex = scripts.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			if (typeIndex == -1) continue;
			const std::vector<ScriptProperty>& properties = scripts.ScriptProperties(typeIndex);
			for (size_t i = 0; i < properties.size(); ++i)
			{
				if (properties[i].m_TypeHash != ST_Asset) continue;
				const size_t offset = properties[i].m_RelativeOffset;
				if (offset + sizeof(uint64_t) >= scriptComponent.m_ScriptData.m_Buffer.size()) continue;
				const ResourceReferenceBase* pReferenceMember =
					reinterpret_cast<const ResourceReferenceBase*>(&scriptComponent.m_ScriptData.m_Buffer[offset]);
				if (pReferenceMember->GetUUID()) references.push_back(pReferenceMember->GetUUID());
			}
		}
	}

	void MonoScriptedManager::OnBodyActivated(IEngine* pEngine, UUID sceneID, UUID entityUUID)
	{
		GScene* pScene = pEngine->GetSceneManager()->GetOpenScene(sceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(entityUUID);
		if (!entity.IsValid()) return;
		if (!entity.HasComponent<MonoScriptComponent>()) return;
		MonoScriptComponent& scriptComponent = entity.GetComponent<MonoScriptComponent>();
		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;
		MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnBodyActivated", nullptr);
	}

	void MonoScriptedManager::OnBodyDeactivated(IEngine* pEngine, UUID sceneID, UUID entityUUID)
	{
		GScene* pScene = pEngine->GetSceneManager()->GetOpenScene(sceneID);
		if (!pScene) return;
		Entity entity = pScene->GetEntityByUUID(entityUUID);
		if (!entity.IsValid()) return;
		if (!entity.HasComponent<MonoScriptComponent>()) return;
		MonoScriptComponent& scriptComponent = entity.GetComponent<MonoScriptComponent>();
		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;
		MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
		if (!pScriptObject) return;
		scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnBodyDeactivated", nullptr);
	}

	void MonoScriptedManager::OnContactAdded(IEngine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID)
	{
		GScene* pScene1 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene1) return;
		Entity entity1 = pScene1->GetEntityByUUID(entity1UUID);
		if (!entity1.IsValid()) return;
		GScene* pScene2 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene2) return;
		Entity entity2 = pScene2->GetEntityByUUID(entity2UUID);
		if (!entity2.IsValid()) return;

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		MonoObject* pMonoObject1 = pCoreLibManager->CreateSceneObject(entity1UUID, scene1ID);
		MonoObject* pMonoObject2 = pCoreLibManager->CreateSceneObject(entity2UUID, scene2ID);
		if (!pMonoObject1 || !pMonoObject2) return;

		if (entity1.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity1.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject2
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactAdded", args.data());
			}
		}

		if (entity2.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity2.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject1
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactAdded", args.data());
			}
		}
	}

	void MonoScriptedManager::OnContactPersisted(IEngine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID)
	{
		GScene* pScene1 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene1) return;
		Entity entity1 = pScene1->GetEntityByUUID(entity1UUID);
		if (!entity1.IsValid()) return;
		GScene* pScene2 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene2) return;
		Entity entity2 = pScene2->GetEntityByUUID(entity2UUID);
		if (!entity2.IsValid()) return;

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		MonoObject* pMonoObject1 = pCoreLibManager->CreateSceneObject(entity1UUID, scene1ID);
		MonoObject* pMonoObject2 = pCoreLibManager->CreateSceneObject(entity2UUID, scene2ID);
		if (!pMonoObject1 || !pMonoObject2) return;

		if (entity1.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity1.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject2
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactPersisted", args.data());
			}
		}

		if (entity2.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity2.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject1
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactPersisted", args.data());
			}
		}
	}

	void MonoScriptedManager::OnContactRemoved(IEngine* pEngine, UUID scene1ID, UUID entity1UUID, UUID scene2ID, UUID entity2UUID)
	{
		GScene* pScene1 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene1) return;
		Entity entity1 = pScene1->GetEntityByUUID(entity1UUID);
		if (!entity1.IsValid()) return;
		GScene* pScene2 = pEngine->GetSceneManager()->GetOpenScene(scene1ID);
		if (!pScene2) return;
		Entity entity2 = pScene2->GetEntityByUUID(entity2UUID);
		if (!entity2.IsValid()) return;

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		MonoScriptManager& scriptManager = pCoreLibManager->ScriptManager();
		MonoObject* pMonoObject1 = pCoreLibManager->CreateSceneObject(entity1UUID, scene1ID);
		MonoObject* pMonoObject2 = pCoreLibManager->CreateSceneObject(entity2UUID, scene2ID);
		if (!pMonoObject1 || !pMonoObject2) return;

		if (entity1.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity1.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject2
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactRemoved", args.data());
			}
		}

		if (entity2.HasComponent<MonoScriptComponent>())
		{
			MonoScriptComponent& scriptComponent = entity2.GetComponent<MonoScriptComponent>();
			int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
			MonoObject* pScriptObject = mono_gchandle_get_target(scriptComponent.m_ScriptObjectHandle);
			if (typeIndex != -1 && pScriptObject)
			{
				std::vector<void*> args = {
					pMonoObject1
				};
				scriptManager.Invoke((size_t)typeIndex, pScriptObject, "OnContactRemoved", args.data());
			}
		}
	}

	void MonoScriptedManager::OnInitialize()
	{
		Bind(DoStart, &MonoScriptedManager::OnStartImpl);
		Bind(DoStop, &MonoScriptedManager::OnStopImpl);
		Bind(DoValidate, &MonoScriptedManager::OnValidateImpl);
		Bind(DoOnActivate, &MonoScriptedManager::OnEnableImpl);
		Bind(DoOnDeactivate, &MonoScriptedManager::OnDisableImpl);
		Bind(DoUpdate, &MonoScriptedManager::OnUpdateImpl);
		Bind(DoDraw, &MonoScriptedManager::OnDrawImpl);
		Bind(DoGetReferences, &MonoScriptedManager::GetReferencesImpl);
	}
}
