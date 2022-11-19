#include "EntityCSAPI.h"
#include <MonoObjectManager.h>
#include <EntitySceneObject.h>
#include <ComponentTypes.h>
#include <Components.h>
#include <CoreCSAPI.h>
#include <LayerManager.h>

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
		return pEntityScene->GetRegistry()->GetComponent<T>(pEntityHandle->m_EntityID);
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

#pragma region MeshFilter



#pragma endregion

#pragma region MeshRenderer



#pragma endregion

#pragma region CameraComponent
	float CameraComponent_GetHalfFOV(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_HalfFOV;
	}

	void CameraComponent_SetHalfFOV(MonoEntityHandle* pEntityHandle, UUID componentID, float halfFov)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_HalfFOV = halfFov;
	}

	float CameraComponent_GetNear(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_Near;
	}

	void CameraComponent_SetNear(MonoEntityHandle* pEntityHandle, UUID componentID, float near)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_Near = near;
	}

	float CameraComponent_GetFar(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_Far;
	}

	void CameraComponent_SetFar(MonoEntityHandle* pEntityHandle, UUID componentID, float far)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_Far = far;
	}

	int CameraComponent_GetDisplayIndex(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_DisplayIndex;
	}

	void CameraComponent_SetDisplayIndex(MonoEntityHandle* pEntityHandle, UUID componentID, int displayIndex)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_DisplayIndex = displayIndex;
	}

	int CameraComponent_GetPriority(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_Priority;
	}

	void CameraComponent_SetPriority(MonoEntityHandle* pEntityHandle, UUID componentID, int priority)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_Priority = priority;
	}

	LayerMask CameraComponent_GetLayerMask(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_LayerMask;
	}

	void CameraComponent_SetLayerMask(MonoEntityHandle* pEntityHandle, UUID componentID, LayerMask* pLayerMask)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_LayerMask.m_Mask = pLayerMask->m_Mask;
	}

	glm::vec4 CameraComponent_GetClearColor(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_ClearColor;
	}

	void CameraComponent_SetClearColor(MonoEntityHandle* pEntityHandle, UUID componentID, glm::vec4* clearCol)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_ClearColor = *clearCol;
	}

	uint64_t CameraComponent_GetCameraID(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_Camera.GetUUID();
	}
#pragma endregion

#pragma region Layer Component
	LayerWrapper LayerComponent_GetLayer(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		LayerComponent& layerComp = GetComponent<LayerComponent>(pEntityHandle, componentID);
		return LayerWrapper(layerComp.m_Layer.Layer());
	}

	void LayerComponent_SetLayer(MonoEntityHandle* pEntityHandle, UUID componentID, LayerWrapper* layer)
	{
		LayerComponent& layerComp = GetComponent<LayerComponent>(pEntityHandle, componentID);
		const Layer* pLayer = LayerManager::GetLayerByName(mono_string_to_utf8(layer->Name));
		layerComp.m_Layer.m_LayerName = pLayer ? pLayer->m_Name : "";
	}
#pragma endregion

#pragma region Light Component
	glm::vec4 LightComponent_GetColor(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		return lightComp.m_Color;
	}
	
	void LightComponent_SetColor(MonoEntityHandle* pEntityHandle, UUID componentID, glm::vec4* color)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		lightComp.m_Color = *color;
	}

	float LightComponent_GetIntensity(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		return lightComp.m_Intensity;
	}

	void LightComponent_SetIntensity(MonoEntityHandle* pEntityHandle, UUID componentID, float intensity)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		lightComp.m_Intensity = intensity;
	}

	float LightComponent_GetRange(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		return lightComp.m_Range;
	}

	void LightComponent_SetRange(MonoEntityHandle* pEntityHandle, UUID componentID, float range)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		lightComp.m_Range = range;
	}
#pragma endregion

#pragma region Binding
	void EntityCSAPI::GetInternallCalls(std::vector<InternalCall>& internalCalls)
	{
		// Entity
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Entity::IsValid()", IsValid));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.EntityBehaviour::GetEntityHandle()", GetEntityHandle));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.EntityComponentManager::Component_GetComponentID", Component_GetComponentID));

		// Transform
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetLocalPosition", Transform_GetLocalPosition));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_SetLocalPosition", Transform_SetLocalPosition));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetLocalRotation", Transform_GetLocalRotation));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_SetLocalRotation", Transform_SetLocalRotation));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetLocalRotationEuler", Transform_GetLocalRotationEuler));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_SetLocalRotationEuler", Transform_SetLocalRotationEuler));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetLocalScale", Transform_GetLocalScale));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_SetLocalScale", Transform_SetLocalScale));

		// Camera
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_GetHalfFOV", CameraComponent_GetHalfFOV));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_SetHalfFOV", CameraComponent_SetHalfFOV));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_GetNear", CameraComponent_GetNear));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_SetNear", CameraComponent_SetNear));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_GetFar", CameraComponent_GetFar));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_SetFar", CameraComponent_SetFar));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_GetDisplayIndex", CameraComponent_GetDisplayIndex));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_SetDisplayIndex", CameraComponent_SetDisplayIndex));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_GetPriority", CameraComponent_GetPriority));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_SetPriority", CameraComponent_SetPriority));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_GetLayerMask", CameraComponent_GetLayerMask));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_SetLayerMask", CameraComponent_SetLayerMask));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_GetClearColor", CameraComponent_GetClearColor));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_SetClearColor", CameraComponent_SetClearColor));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.CameraComponent::CameraComponent_GetCameraID", CameraComponent_GetCameraID));

		// Layer
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.LayerComponent::LayerComponent_GetLayer", LayerComponent_GetLayer));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.LayerComponent::LayerComponent_SetLayer", LayerComponent_SetLayer));

		// Light
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.LightComponent::LightComponent_GetColor", LightComponent_GetColor));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.LightComponent::LightComponent_SetColor", LightComponent_SetColor));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.LightComponent::LightComponent_GetIntensity", LightComponent_GetIntensity));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.LightComponent::LightComponent_SetIntensity", LightComponent_SetIntensity));

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.LightComponent::LightComponent_GetRange", LightComponent_GetRange));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.LightComponent::LightComponent_SetRange", LightComponent_SetRange));
	}

	MonoEntityHandle::MonoEntityHandle() : m_EntityID(0), m_SceneID(0)
	{
	}

	MonoEntityHandle::MonoEntityHandle(uint64_t entityID, uint64_t sceneID) : m_EntityID(entityID), m_SceneID(sceneID)
	{
	}
#pragma endregion
}
