#include "EntityCSAPI.h"
#include <MonoScriptObjectManager.h>
#include <EntitySceneObject.h>
#include <ComponentTypes.h>
#include <Components.h>
#include <CoreCSAPI.h>
#include <LayerManager.h>
#include <MonoAssetManager.h>
#include <MathCSAPI.h>

namespace Glory
{
#pragma region Entity

	EntityScene* GetEntityScene(UUID sceneID)
	{
		if (sceneID == 0) return nullptr;
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(sceneID);
		if (pScene == nullptr) return nullptr;
		EntityScene* pEntityScene = (EntityScene*)pScene;
		return pEntityScene;
	}

	EntityScene* GetEntityScene(MonoEntityHandle* pEntityHandle)
	{
		if (pEntityHandle->m_EntityID == 0) return nullptr;
		return GetEntityScene((UUID)pEntityHandle->m_SceneID);
	}

	template<typename T>
	static T& GetComponent(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		EntityScene* pEntityScene = GetEntityScene(pEntityHandle);
		EntityView* pEntityView = pEntityScene->GetRegistry()->GetEntityView(pEntityHandle->m_EntityID);
		uint32_t hash = pEntityView->ComponentType(componentID);
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
		Object* pEngineObject = MonoScriptObjectManager::GetScriptObject(pObject);
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

	void Transform_SetLocalRotationEuler(MonoEntityHandle* pEntityHandle, UUID componentID, Vec3Wrapper* rotation)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Rotation = glm::quat(ToGLMVec3(*rotation));
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

	glm::vec3 Transform_GetForward(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return glm::vec3(transform.MatTransform[2][0], transform.MatTransform[2][1], transform.MatTransform[2][2]);
	}

	glm::vec3 Transform_GetUp(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return glm::vec3(transform.MatTransform[1][0], transform.MatTransform[1][1], transform.MatTransform[1][2]);
	}

	void Transform_SetForward(MonoEntityHandle* pEntityHandle, UUID componentID, glm::vec3* forward)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Rotation = glm::conjugate(glm::quatLookAt(*forward, { 0.0f, 1.0f, 0.0f }));
	}

	glm::vec3 Transform_GetRight(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return glm::vec3(transform.MatTransform[0][0], transform.MatTransform[0][1], transform.MatTransform[0][2]);
	}

	Mat4Wrapper Transform_GetWorld(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return ToMat4Wrapper(transform.MatTransform);
	}

#pragma endregion

#pragma region MeshFilter

	MonoObject* MeshFilter_GetMesh(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		MeshFilter& meshFilter = GetComponent<MeshFilter>(pEntityHandle, componentID);
		UUID uuid = meshFilter.m_Mesh.AssetUUID();
		return MonoAssetManager::MakeMonoAssetObject<ModelData>(uuid);
	}

	void MeshFilter_SetMesh(MonoEntityHandle* pEntityHandle, UUID componentID, UUID modelID)
	{
		MeshFilter& meshFilter = GetComponent<MeshFilter>(pEntityHandle, componentID);
		meshFilter.m_Mesh.SetUUID(modelID);
	}

#pragma endregion

#pragma region MeshRenderer

	MonoObject* MeshRenderer_GetMaterial(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(pEntityHandle, componentID);
		const UUID uuid = meshRenderer.m_Material.AssetUUID();
		return MonoAssetManager::MakeMonoAssetObject<MaterialData>(uuid);
	}

	void MeshRenderer_SetMaterial(MonoEntityHandle* pEntityHandle, UUID componentID, UUID materialID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Material = materialID;
	}

	MonoObject* MeshRenderer_GetMesh(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(pEntityHandle, componentID);
		const UUID uuid = meshRenderer.m_Mesh.AssetUUID();
		return MonoAssetManager::MakeMonoAssetObject<MeshData>(uuid);
	}

	void MeshRenderer_SetMesh(MonoEntityHandle* pEntityHandle, UUID componentID, UUID meshID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Mesh = meshID;
	}

#pragma endregion

#pragma region ModelRenderer

	MonoObject* ModelRenderer_GetMaterial(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		UUID uuid = meshRenderer.m_Materials.size() > 0 ? meshRenderer.m_Materials[0].m_MaterialReference.AssetUUID() : 0;
		return MonoAssetManager::MakeMonoAssetObject<MaterialData>(uuid);
	}

	void ModelRenderer_SetMaterial(MonoEntityHandle* pEntityHandle, UUID componentID, UUID materialID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		if (meshRenderer.m_Materials.size() <= 0) meshRenderer.m_Materials.push_back(materialID);
		else meshRenderer.m_Materials[0] = materialID;
	}

	size_t ModelRenderer_GetMaterialCount(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		return meshRenderer.m_Materials.size();
	}

	MonoObject* ModelRenderer_GetMaterialAt(MonoEntityHandle* pEntityHandle, UUID componentID, size_t index)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		if (meshRenderer.m_Materials.size() <= index) return nullptr;
		UUID uuid = meshRenderer.m_Materials[index].m_MaterialReference.AssetUUID();
		return MonoAssetManager::MakeMonoAssetObject<MaterialData>(uuid);
	}

	void ModelRenderer_AddMaterial(MonoEntityHandle* pEntityHandle, UUID componentID, UUID materialID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Materials.push_back(materialID);
	}

	void ModelRenderer_SetMaterialAt(MonoEntityHandle* pEntityHandle, UUID componentID, size_t index, UUID materialID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		if (meshRenderer.m_Materials.size() <= index) return;
		meshRenderer.m_Materials[index] = materialID;
	}

	void ModelRenderer_ClearMaterials(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Materials.clear();
	}

	MonoObject* ModelRenderer_GetModel(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		const UUID uuid = meshRenderer.m_Model.AssetUUID();
		return MonoAssetManager::MakeMonoAssetObject<ModelData>(uuid);
	}

	void ModelRenderer_SetModel(MonoEntityHandle* pEntityHandle, UUID componentID, UUID modelID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Model = modelID;
	}

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
		layerComp.m_Layer = pLayer ? LayerManager::GetLayerIndex(pLayer) + 1 : 0;
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

#pragma region Physics Component

#define PHYSICS Game::GetGame().GetEngine()->GetPhysicsModule()

#pragma region States

	void PhysicsBody_Activate(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->ActivateBody(physicsComp.m_BodyID);
	}

	void PhysicsBody_Deactivate(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->DeactivateBody(physicsComp.m_BodyID);
	}

	bool PhysicsBody_IsActive(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->IsBodyActive(physicsComp.m_BodyID);
	}

	bool PhysicsBody_IsValid(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->IsValidBody(physicsComp.m_BodyID);
	}

#pragma endregion

#pragma region Position and rotation

	void PhysicsBody_SetPosition(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* position, const ActivationType activationType)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyPosition(physicsComp.m_BodyID, ToGLMVec3(*position), activationType);
	}

	void PhysicsBody_SetRotation(MonoEntityHandle* pEntityHandle, UUID componentID, const QuatWrapper* rotation, const ActivationType activationType)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyRotation(physicsComp.m_BodyID, ToGLMQuat(*rotation), activationType);
	}

	void PhysicsBody_SetScale(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* scale, const ActivationType activationType)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyScale(physicsComp.m_BodyID, ToGLMVec3(*scale), activationType);
	}

	Vec3Wrapper PhysicsBody_GetPosition(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyPosition(physicsComp.m_BodyID);
	}

	Vec3Wrapper PhysicsBody_GetCenterOfMassPosition(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyCenterOfMassPosition(physicsComp.m_BodyID);
	}

	QuatWrapper PhysicsBody_GetRotation(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyRotation(physicsComp.m_BodyID);
	}

#pragma endregion

#pragma region Velocities

	void PhysicsBody_MoveKinematic(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* targetPosition, const QuatWrapper* targetRotation, float deltaTime)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->MoveBodyKinematic(physicsComp.m_BodyID, ToGLMVec3(*targetPosition), ToGLMQuat(*targetRotation), deltaTime);
	}

	void PhysicsBody_SetLinearAndAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyLinearAndAngularVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void PhysicsBody_GetLinearAndAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, Vec3Wrapper* linearVelocity, Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		glm::vec3 lv, av;
		PHYSICS->GetBodyLinearAndAngularVelocity(physicsComp.m_BodyID, lv, av);
		*linearVelocity = lv;
		*angularVelocity = av;
	}

	void PhysicsBody_SetLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyLinearVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity));
	}

	Vec3Wrapper PhysicsBody_GetLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyLinearVelocity(physicsComp.m_BodyID);
	}

	void PhysicsBody_AddLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyLinearVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity));
	}

	void PhysicsBody_AddLinearAndAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyLinearAndAngularVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void PhysicsBody_SetAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyAngularVelocity(physicsComp.m_BodyID, ToGLMVec3(*angularVelocity));
	}

	Vec3Wrapper PhysicsBody_GetAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyAngularVelocity(physicsComp.m_BodyID);
	}

	Vec3Wrapper PhysicsBody_GetPointVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* point)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyPointVelocity(physicsComp.m_BodyID, ToGLMVec3(*point));
	}

	void PhysicsBody_SetPositionRotationAndVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* position, const QuatWrapper* rotation, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyPositionRotationAndVelocity(physicsComp.m_BodyID, ToGLMVec3(*position), ToGLMQuat(*rotation), ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

#pragma endregion

#pragma region Forces

	void PhysicsBody_AddForce(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* force)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyForce(physicsComp.m_BodyID, ToGLMVec3(*force));
	}

	void PhysicsBody_AddForce_Point(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* force, const Vec3Wrapper* point)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyForce(physicsComp.m_BodyID, ToGLMVec3(*force), ToGLMVec3(*point));
	}

	void PhysicsBody_AddTorque(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* torque)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyTorque(physicsComp.m_BodyID, ToGLMVec3(*torque));
	}

	void PhysicsBody_AddForceAndTorque(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* force, const Vec3Wrapper* torque)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyForceAndTorque(physicsComp.m_BodyID, ToGLMVec3(*force), ToGLMVec3(*torque));
	}

#pragma endregion

#pragma region Impulses

	void PhysicsBody_AddImpulse(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* impulse)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyImpulse(physicsComp.m_BodyID, ToGLMVec3(*impulse));
	}

	void PhysicsBody_AddImpulse_Point(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* impulse, const Vec3Wrapper* point)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyForce(physicsComp.m_BodyID, ToGLMVec3(*impulse), ToGLMVec3(*point));
	}

	void PhysicsBody_AddAngularImpulse(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* angularImpulse)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyAngularImpulse(physicsComp.m_BodyID, ToGLMVec3(*angularImpulse));
	}

#pragma endregion

#pragma region EntitySceneObject

	MonoEntityHandle EntitySceneObject_GetEntityHandle(uint64_t objectID, uint64_t sceneID)
	{
		EntityScene* pScene = GetEntityScene(sceneID);
		if(!pScene) return MonoEntityHandle();
		SceneObject* pSceneObject = pScene->FindSceneObject(objectID);
		if (!pSceneObject) return MonoEntityHandle();
		EntitySceneObject* pEntityObject = (EntitySceneObject*)pSceneObject;
		if (!pEntityObject) return MonoEntityHandle();
		Entity entityHandle = pEntityObject->GetEntityHandle();
		EntityID entityID = entityHandle.GetEntityID();
		return MonoEntityHandle(entityID, pScene->GetUUID());
	}

#pragma endregion

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

		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetForward", Transform_GetForward));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_SetForward", Transform_SetForward));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetRight", Transform_GetRight));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.Transform::Transform_GetUp", Transform_GetUp));

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

		// MeshFilter
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshFilter::MeshFilter_GetMesh", MeshFilter_GetMesh));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshFilter::MeshFilter_SetMesh", MeshFilter_SetMesh));

		// MeshRenderer
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::MeshRenderer_GetMaterial", MeshRenderer_GetMaterial));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::MeshRenderer_SetMaterial", MeshRenderer_SetMaterial));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::MeshRenderer_GetMesh", MeshRenderer_GetMesh));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::MeshRenderer_SetMesh", MeshRenderer_SetMesh));

		// ModelRenderer
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::ModelRenderer_GetMaterial", ModelRenderer_GetMaterial));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::ModelRenderer_SetMaterial", ModelRenderer_SetMaterial));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::ModelRenderer_GetMaterialCount", ModelRenderer_GetMaterialCount));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::ModelRenderer_GetMaterialAt", ModelRenderer_GetMaterialAt));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::ModelRenderer_AddMaterial", ModelRenderer_AddMaterial));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::ModelRenderer_SetMaterialAt", ModelRenderer_SetMaterialAt));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::ModelRenderer_ClearMaterials", ModelRenderer_ClearMaterials));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::ModelRenderer_GetModel", ModelRenderer_GetModel));
		internalCalls.push_back(InternalCall("csharp", "GloryEngine.Entities.MeshRenderer::ModelRenderer_SetModel", ModelRenderer_SetModel));

		/*  status */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_Activate", PhysicsBody_Activate);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_Deactivate", PhysicsBody_Deactivate);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_IsActive", PhysicsBody_IsActive);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_IsValid", PhysicsBody_IsValid);

		/* Position and rotation */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetPosition", PhysicsBody_SetPosition);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetRotation", PhysicsBody_SetRotation);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetScale", PhysicsBody_SetScale);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetPosition", PhysicsBody_GetPosition);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetCenterOfMassPosition", PhysicsBody_GetCenterOfMassPosition);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetRotation", PhysicsBody_GetRotation);

		/* Velocities */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_MoveKinematic", PhysicsBody_MoveKinematic);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetLinearAndAngularVelocity", PhysicsBody_SetLinearAndAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetLinearAndAngularVelocity", PhysicsBody_GetLinearAndAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetLinearVelocity", PhysicsBody_SetLinearVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetLinearVelocity", PhysicsBody_GetLinearVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddLinearVelocity", PhysicsBody_AddLinearVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddLinearAndAngularVelocity", PhysicsBody_AddLinearAndAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetAngularVelocity", PhysicsBody_SetAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetAngularVelocity", PhysicsBody_GetAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetPointVelocity", PhysicsBody_GetPointVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetPositionRotationAndVelocity", PhysicsBody_SetPositionRotationAndVelocity);

		/* Velocities */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddForce", PhysicsBody_AddForce);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddForce_Point", PhysicsBody_AddForce_Point);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddTorque", PhysicsBody_AddTorque);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddForceAndTorque", PhysicsBody_AddForceAndTorque);

		/* Impulses */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddImpulse", PhysicsBody_AddImpulse);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddImpulse_Point", PhysicsBody_AddImpulse_Point);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddAngularImpulse", PhysicsBody_AddAngularImpulse);

		/* Entity Scene Object */
		BIND("GloryEngine.Entities.EntitySceneObject::EntitySceneObject_GetEntityHandle", EntitySceneObject_GetEntityHandle);
	}

	MonoEntityHandle::MonoEntityHandle() : m_EntityID(0), m_SceneID(0)
	{
	}

	MonoEntityHandle::MonoEntityHandle(uint64_t entityID, uint64_t sceneID) : m_EntityID(entityID), m_SceneID(sceneID)
	{
	}

#pragma endregion
}
