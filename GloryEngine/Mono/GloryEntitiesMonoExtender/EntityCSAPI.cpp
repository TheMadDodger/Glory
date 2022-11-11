#include "EntityCSAPI.h"
#include <MonoObjectManager.h>
#include <EntitySceneObject.h>
#include <ComponentTypes.h>
#include <Components.h>

namespace Glory
{
#pragma region Entity
	EntityScene* GetEntityScene(MonoEntityHandle* pEntityHandle)
	{
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return nullptr;
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return nullptr;
		EntityScene* pEntityScene = (EntityScene*)pScene;
		return pEntityScene;
	}

	template<typename T>
	static T& GetComponent(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		EntityScene* pEntityScene = GetEntityScene(pEntityHandle);
		EntityView* pEntityView = pEntityScene->GetRegistry()->GetEntityView(pEntityHandle->m_EntityID);
		size_t hash = pEntityView->ComponentType(componentID);
		return pEntityScene->GetRegistry()->GetComponent<Transform>(pEntityHandle->m_EntityID);
	}

	bool IsValid(MonoEntityHandle* pMonoEntityHandle)
	{
		if (pMonoEntityHandle->m_EntityID == 0 || pMonoEntityHandle->m_SceneID == 0) return false;
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene((UUID)pMonoEntityHandle->m_SceneID);
		if (pScene == nullptr) return false;
		EntityScene* pEntityScene = (EntityScene*)pScene;
		if (pEntityScene == nullptr) return false;
		return pEntityScene->GetRegistry()->IsValid(pMonoEntityHandle->m_EntityID);

		//MonoClass* pClass = mono_object_get_class(pObject);
		//MonoClassField* pField = mono_class_get_field_from_name(pClass, "_entityID");
		//uint32_t entID;
		//mono_field_get_value(pObject, pField, &entID);
		//Object* pObject = MonoObjectManager::GetObject(pObject);
		//EntitySceneObject* pEntityObject = (EntitySceneObject*)pObject;
		//EntityScene* pScene = (EntityScene*)pEntityObject->GetScene();
		//return pScene->GetRegistry()->IsValid(entID);
	}

	MonoEntityHandle GetEntityHandle(MonoObject* pObject)
	{
		Object* pEngineObject = MonoObjectManager::GetObject(pObject);
		if (!pEngineObject) return MonoEntityHandle();
		EntitySceneObject* pEntityObject = (EntitySceneObject*)pEngineObject;
		if (!pEntityObject) return MonoEntityHandle();
		Entity entityHandle = pEntityObject->GetEntityHandle();
		EntityID entityID = entityHandle.GetEntityID();
		EntityScene* pScene = entityHandle.GetScene();
		return MonoEntityHandle(entityID, pScene->GetUUID());
	}

	uint64_t Component_GetComponentID(MonoEntityHandle* pEntityHandle, MonoString* pComponentName)
	{
		const std::string componentName{ mono_string_to_utf8(pComponentName) };
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return 0;
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return 0;
		EntityScene* pEntityScene = (EntityScene*)pScene;
		if (pEntityScene == nullptr) return 0;
		const size_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		EntityView* pEntityView = pEntityScene->GetRegistry()->GetEntityView(pEntityHandle->m_EntityID);
		
		for (auto iter = pEntityView->GetIterator(); iter != pEntityView->GetIteratorEnd(); iter++)
		{
			if (iter->second != componentHash) continue;
			return iter->first;
		}
		return 0;
	}
#pragma endregion

#pragma region Transform
	glm::vec3 Transform_GetLocalPosition(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return transform.Position;
	}

	void Transform_SetLocalPosition(MonoEntityHandle* pEntityHandle, UUID componentID, glm::vec3* position)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Position = *position;
	}

	glm::quat Transform_GetLocalRotation(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return transform.Rotation;
	}

	void Transform_SetLocalRotation(MonoEntityHandle* pEntityHandle, UUID componentID, glm::quat* rotation)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Rotation = *rotation;
	}

	glm::vec3 Transform_GetLocalRotationEuler(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return glm::eulerAngles(transform.Rotation);
	}

	void Transform_SetLocalRotationEuler(MonoEntityHandle* pEntityHandle, UUID componentID, glm::vec3* rotation)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Rotation = glm::quat(*rotation);
	}

	glm::vec3 Transform_GetLocalScale(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return transform.Scale;
	}

	void Transform_SetLocalScale(MonoEntityHandle* pEntityHandle, UUID componentID, glm::vec3* scale)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Scale = *scale;
	}
#pragma endregion

#pragma region Binding
	void EntityCSAPI::GetInternallCalls(std::vector<InternalCall>& internalCalls)
	{
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Entity::IsValid()", IsValid));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.EntityBehaviour::GetEntityHandle()", GetEntityHandle));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.EntityComponentManager::Component_GetComponentID", Component_GetComponentID));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetLocalPosition", Transform_GetLocalPosition));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_SetLocalPosition", Transform_SetLocalPosition));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetLocalRotation", Transform_GetLocalRotation));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_SetLocalRotation", Transform_SetLocalRotation));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetLocalRotationEuler", Transform_GetLocalRotationEuler));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_SetLocalRotationEuler", Transform_SetLocalRotationEuler));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetLocalScale", Transform_GetLocalScale));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_SetLocalScale", Transform_SetLocalScale));
	}

	MonoEntityHandle::MonoEntityHandle() : m_EntityID(0), m_SceneID(0)
	{
	}

	MonoEntityHandle::MonoEntityHandle(uint64_t entityID, uint64_t sceneID) : m_EntityID(entityID), m_SceneID(sceneID)
	{
	}
#pragma endregion
}
