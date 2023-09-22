#include "EntityCSAPI.h"
#include "AssemblyDomain.h"

#include <ScenesModule.h>
#include <PhysicsModule.h>
#include <MonoSceneManager.h>
#include <MonoSceneObjectManager.h>
#include <MonoScriptObjectManager.h>
#include <EntitySceneObject.h>
#include <ComponentTypes.h>
#include <Components.h>
#include <CoreCSAPI.h>
#include <LayerManager.h>
#include <CharacterManager.h>
#include <ShapeManager.h>
#include <MonoAssetManager.h>
#include <MathCSAPI.h>

namespace Glory
{
#pragma region Entity

	EntityScene* GetEntityScene(UUID sceneID)
	{
		if (sceneID == 0) return nullptr;
		GScene* pScene = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>()->GetOpenScene(sceneID);
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

	bool Entity_IsValid(MonoEntityHandle* pMonoEntityHandle)
	{
		if (pMonoEntityHandle->m_EntityID == 0 || pMonoEntityHandle->m_SceneID == 0) return false;
		GScene* pScene = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>()->GetOpenScene((UUID)pMonoEntityHandle->m_SceneID);
		if (pScene == nullptr) return false;
		EntityScene* pEntityScene = (EntityScene*)pScene;
		if (pEntityScene == nullptr) return false;
		return pEntityScene->GetRegistry()->IsValid(pMonoEntityHandle->m_EntityID);
	}

	MonoObject* Entity_GetSceneObjectID(MonoEntityHandle* pMonoEntityHandle)
	{
		EntityScene* pScene = (EntityScene*)Game::GetGame().GetEngine()->GetMainModule<ScenesModule>()->GetOpenScene(pMonoEntityHandle->m_SceneID);
		if (!pScene) return nullptr;
		MonoSceneObjectManager* pObjectManager = MonoSceneManager::GetSceneObjectManager(pScene);
		if (!pObjectManager) return nullptr;
		EntitySceneObject* pObject = pScene->GetEntitySceneObjectFromEntityID(pMonoEntityHandle->m_EntityID);
		if (!pObject) return nullptr;
		return pObjectManager->GetSceneObject(pObject);
	}

	MonoEntityHandle GetEntityHandle(MonoObject* pObject)
	{
		AssemblyDomain* pDomain = MonoManager::Instance()->ActiveDomain();
		Object* pEngineObject = MonoManager::Instance()->ActiveDomain()->ScriptObjectManager()->GetScriptObject(pObject);
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
		GScene* pScene = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return 0;
		EntityScene* pEntityScene = (EntityScene*)pScene;
		if (pEntityScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		EntityView* pEntityView = pEntityScene->GetRegistry()->GetEntityView(pEntityHandle->m_EntityID);

		for (auto iter = pEntityView->GetIterator(); iter != pEntityView->GetIteratorEnd(); iter++)
		{
			if (iter->second != componentHash) continue;
			return iter->first;
		}
		return 0;
	}

	uint64_t Component_AddComponent(MonoEntityHandle* pEntityHandle, MonoString* pComponentName)
	{
		const std::string componentName{ mono_string_to_utf8(pComponentName) };
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return 0;
		GScene* pScene = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return 0;
		EntityScene* pEntityScene = (EntityScene*)pScene;
		if (pEntityScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		const UUID uuid{};
		pEntityScene->GetRegistry()->CreateComponent(pEntityHandle->m_EntityID, componentHash, uuid);
		return uuid;
	}

	uint64_t Component_RemoveComponent(MonoEntityHandle* pEntityHandle, MonoString* pComponentName)
	{
		const std::string componentName{ mono_string_to_utf8(pComponentName) };
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return 0;
		GScene* pScene = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return 0;
		EntityScene* pEntityScene = (EntityScene*)pScene;
		if (pEntityScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		return pEntityScene->GetRegistry()->RemoveComponent(pEntityHandle->m_EntityID, componentHash);
	}

	void Component_RemoveComponentByID(MonoEntityHandle* pEntityHandle, uint64_t id)
	{
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return;
		GScene* pScene = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return;
		EntityScene* pEntityScene = (EntityScene*)pScene;
		if (pEntityScene == nullptr) return;
		EntityView* pEntityView = pEntityScene->GetRegistry()->GetEntityView(pEntityHandle->m_EntityID);
		const uint32_t hash = pEntityView->ComponentType(id);
		pEntityScene->GetRegistry()->RemoveComponent(pEntityHandle->m_EntityID, hash);
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

	QuatWrapper Transform_GetLocalRotation(MonoEntityHandle* pEntityHandle, UUID componentID)
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

#define PHYSICS Game::GetGame().GetEngine()->GetMainModule<PhysicsModule>()

#pragma region States

	uint32_t PhysicsBody_GetID(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return physicsComp.m_BodyID;
	}

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

#pragma endregion

#pragma region Character Controller

#define CHARACTERS PHYSICS->GetCharacterManager()
#define SHAPES PHYSICS->GetShapeManager()

#pragma region States

	uint32_t CharacterController_GetCharacterID(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return characterController.m_CharacterID;
	}

	uint32_t CharacterController_GetBodyID(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetBodyID(characterController.m_CharacterID);
	}

	void CharacterController_Activate(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->Activate(characterController.m_CharacterID);
	}

	void CharacterController_Deactivate(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		const uint32_t bodyID = CHARACTERS->GetBodyID(characterController.m_CharacterID);
		PHYSICS->DeactivateBody(bodyID);
	}

	bool CharacterController_IsActive(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		const uint32_t bodyID = CHARACTERS->GetBodyID(characterController.m_CharacterID);
		return PHYSICS->IsBodyActive(bodyID);
	}

#pragma endregion

#pragma region Position and rotation

	void CharacterController_SetPosition(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* position, const ActivationType activationType)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->SetPosition(characterController.m_CharacterID, ToGLMVec3(*position), activationType);
	}

	void CharacterController_SetRotation(MonoEntityHandle* pEntityHandle, UUID componentID, const QuatWrapper* rotation, const ActivationType activationType)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->SetRotation(characterController.m_CharacterID, ToGLMQuat(*rotation), activationType);
	}

	Vec3Wrapper CharacterController_GetPosition(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetPosition(characterController.m_CharacterID);
	}

	Vec3Wrapper CharacterController_GetCenterOfMassPosition(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetCenterOfMassPosition(characterController.m_CharacterID);
	}

	QuatWrapper CharacterController_GetRotation(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetRotation(characterController.m_CharacterID);
	}

#pragma endregion

#pragma region Velocities

	void CharacterController_SetLinearAndAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->SetLinearAndAngularVelocity(characterController.m_CharacterID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void CharacterController_SetLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->SetLinearVelocity(characterController.m_CharacterID, ToGLMVec3(*linearVelocity));
	}

	Vec3Wrapper CharacterController_GetLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetLinearVelocity(characterController.m_CharacterID);
	}

	void CharacterController_AddLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->AddLinearVelocity(characterController.m_CharacterID, ToGLMVec3(*linearVelocity));
	}

#pragma endregion

#pragma region Impulses

	void CharacterController_AddImpulse(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* impulse)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->AddImpulse(characterController.m_CharacterID, ToGLMVec3(*impulse));
	}

#pragma endregion

#pragma region Shapes

	uint64_t CharacterController_GetShapeID(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return characterController.m_ShapeID;
	}

	bool CharacterController_SetShape(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint64_t shapeID, float maxPenetrationDepth = 0.0f, bool lockBodies = true)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		const ShapeData* pShapeData = SHAPES->GetShape(shapeID);
		if (!pShapeData)
		{
			std::stringstream log;
			log << "CharacterController_SetShape: Failed to set shape, shape with ID: " << shapeID << " does not exist!";
			Debug::LogError(log.str());
			return false;
		}

		/* Never continue to store the original shape! Some physics engines like jolt delete them upon swappin! */
		SHAPES->DestroyShape(characterController.m_ShapeID);
		characterController.m_ShapeID = shapeID;
		return CHARACTERS->SetShape(characterController.m_CharacterID, *pShapeData, maxPenetrationDepth, lockBodies);
	}

#pragma endregion

#pragma endregion

#pragma region ScriptedComponent

	MonoObject* ScriptedComponent_GetScript(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		ScriptedComponent& scriptComp = GetComponent<ScriptedComponent>(pEntityHandle, componentID);
		const UUID uuid = scriptComp.m_Script.AssetUUID();
		return MonoAssetManager::MakeMonoAssetObject<ModelData>(uuid);
	}

	void ScriptedComponent_SetScript(MonoEntityHandle* pEntityHandle, UUID componentID, UUID scriptID)
	{
		ScriptedComponent& scriptComp = GetComponent<ScriptedComponent>(pEntityHandle, componentID);
		if (scriptComp.m_Script.AssetUUID() != 0)
		{
			Debug::LogError("You are trying to set the script on a ScriptedComponent that already has a script, this is not allowed.");
			return;
		}
		scriptComp.m_Script = scriptID;
	}

	MonoObject* ScriptedComponent_GetBehaviour(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		ScriptedComponent& scriptComp = GetComponent<ScriptedComponent>(pEntityHandle, componentID);
		MonoScriptObjectManager* pScriptObjectManager = MonoManager::Instance()->ActiveDomain()->ScriptObjectManager();
		/* TODO */
		return nullptr;
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

#pragma region Binding

	void EntityCSAPI::GetInternallCalls(std::vector<InternalCall>& internalCalls)
	{
		/* Entity */
		BIND("GloryEngine.Entities.Entity::Entity_IsValid", Entity_IsValid);
		BIND("GloryEngine.Entities.Entity::Entity_GetSceneObjectID", Entity_GetSceneObjectID);
		BIND("GloryEngine.Entities.EntityBehaviour::GetEntityHandle()", GetEntityHandle);

		BIND("GloryEngine.Entities.EntityComponentManager::Component_GetComponentID", Component_GetComponentID);
		BIND("GloryEngine.Entities.EntityComponentManager::Component_AddComponent", Component_AddComponent);
		BIND("GloryEngine.Entities.EntityComponentManager::Component_RemoveComponent", Component_RemoveComponent);
		BIND("GloryEngine.Entities.EntityComponentManager::Component_RemoveComponentByID", Component_RemoveComponentByID);

		/* Transform */
		BIND("GloryEngine.Entities.Transform::Transform_GetLocalPosition", Transform_GetLocalPosition);
		BIND("GloryEngine.Entities.Transform::Transform_SetLocalPosition", Transform_SetLocalPosition);

		BIND("GloryEngine.Entities.Transform::Transform_GetLocalRotation", Transform_GetLocalRotation);
		BIND("GloryEngine.Entities.Transform::Transform_SetLocalRotation", Transform_SetLocalRotation);

		BIND("GloryEngine.Entities.Transform::Transform_GetLocalRotationEuler", Transform_GetLocalRotationEuler);
		BIND("GloryEngine.Entities.Transform::Transform_SetLocalRotationEuler", Transform_SetLocalRotationEuler);

		BIND("GloryEngine.Entities.Transform::Transform_GetLocalScale", Transform_GetLocalScale);
		BIND("GloryEngine.Entities.Transform::Transform_SetLocalScale", Transform_SetLocalScale);

		BIND("GloryEngine.Entities.Transform::Transform_GetForward", Transform_GetForward);
		BIND("GloryEngine.Entities.Transform::Transform_SetForward", Transform_SetForward);
		BIND("GloryEngine.Entities.Transform::Transform_GetRight", Transform_GetRight);
		BIND("GloryEngine.Entities.Transform::Transform_GetUp", Transform_GetUp);

		/* Camera */
		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetHalfFOV", CameraComponent_GetHalfFOV);
		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_SetHalfFOV", CameraComponent_SetHalfFOV);

		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetNear", CameraComponent_GetNear);
		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_SetNear", CameraComponent_SetNear);

		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetFar", CameraComponent_GetFar);
		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_SetFar", CameraComponent_SetFar);

		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetDisplayIndex", CameraComponent_GetDisplayIndex);
		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_SetDisplayIndex", CameraComponent_SetDisplayIndex);

		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetPriority", CameraComponent_GetPriority);
		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_SetPriority", CameraComponent_SetPriority);

		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetLayerMask", CameraComponent_GetLayerMask);
		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_SetLayerMask", CameraComponent_SetLayerMask);

		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetClearColor", CameraComponent_GetClearColor);
		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_SetClearColor", CameraComponent_SetClearColor);

		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetCameraID", CameraComponent_GetCameraID);

		/* Layer */
		BIND("GloryEngine.Entities.LayerComponent::LayerComponent_GetLayer", LayerComponent_GetLayer);
		BIND("GloryEngine.Entities.LayerComponent::LayerComponent_SetLayer", LayerComponent_SetLayer);

		/* Light */
		BIND("GloryEngine.Entities.LightComponent::LightComponent_GetColor", LightComponent_GetColor);
		BIND("GloryEngine.Entities.LightComponent::LightComponent_SetColor", LightComponent_SetColor);

		BIND("GloryEngine.Entities.LightComponent::LightComponent_GetIntensity", LightComponent_GetIntensity);
		BIND("GloryEngine.Entities.LightComponent::LightComponent_SetIntensity", LightComponent_SetIntensity);

		BIND("GloryEngine.Entities.LightComponent::LightComponent_GetRange", LightComponent_GetRange);
		BIND("GloryEngine.Entities.LightComponent::LightComponent_SetRange", LightComponent_SetRange);

		/* MeshFilter */
		BIND("GloryEngine.Entities.MeshFilter::MeshFilter_GetMesh", MeshFilter_GetMesh);
		BIND("GloryEngine.Entities.MeshFilter::MeshFilter_SetMesh", MeshFilter_SetMesh);

		/* MeshRenderer */
		BIND("GloryEngine.Entities.MeshRenderer::MeshRenderer_GetMaterial", MeshRenderer_GetMaterial);
		BIND("GloryEngine.Entities.MeshRenderer::MeshRenderer_SetMaterial", MeshRenderer_SetMaterial);
		BIND("GloryEngine.Entities.MeshRenderer::MeshRenderer_GetMesh", MeshRenderer_GetMesh);
		BIND("GloryEngine.Entities.MeshRenderer::MeshRenderer_SetMesh", MeshRenderer_SetMesh);

		/* ModelRenderer */
		BIND("GloryEngine.Entities.MeshRenderer::ModelRenderer_GetMaterial", ModelRenderer_GetMaterial);
		BIND("GloryEngine.Entities.MeshRenderer::ModelRenderer_SetMaterial", ModelRenderer_SetMaterial);
		BIND("GloryEngine.Entities.MeshRenderer::ModelRenderer_GetMaterialCount", ModelRenderer_GetMaterialCount);
		BIND("GloryEngine.Entities.MeshRenderer::ModelRenderer_GetMaterialAt", ModelRenderer_GetMaterialAt);
		BIND("GloryEngine.Entities.MeshRenderer::ModelRenderer_AddMaterial", ModelRenderer_AddMaterial);
		BIND("GloryEngine.Entities.MeshRenderer::ModelRenderer_SetMaterialAt", ModelRenderer_SetMaterialAt);
		BIND("GloryEngine.Entities.MeshRenderer::ModelRenderer_ClearMaterials", ModelRenderer_ClearMaterials);
		BIND("GloryEngine.Entities.MeshRenderer::ModelRenderer_GetModel", ModelRenderer_GetModel);
		BIND("GloryEngine.Entities.MeshRenderer::ModelRenderer_SetModel", ModelRenderer_SetModel);

		/* ScriptedComponent */
		BIND("GloryEngine.Entities.ScriptedComponent::ScriptedComponent_GetScript", ScriptedComponent_GetScript);
		BIND("GloryEngine.Entities.ScriptedComponent::ScriptedComponent_SetScript", ScriptedComponent_SetScript);
		BIND("GloryEngine.Entities.ScriptedComponent::ScriptedComponent_GetBehaviour", ScriptedComponent_GetBehaviour);

		/* PhysicsBody */
		/* status */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetID", PhysicsBody_GetID);
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

		/* Forces */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddForce", PhysicsBody_AddForce);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddForce_Point", PhysicsBody_AddForce_Point);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddTorque", PhysicsBody_AddTorque);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddForceAndTorque", PhysicsBody_AddForceAndTorque);

		/* Impulses */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddImpulse", PhysicsBody_AddImpulse);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddImpulse_Point", PhysicsBody_AddImpulse_Point);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddAngularImpulse", PhysicsBody_AddAngularImpulse);

		/* Status */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetCharacterID", CharacterController_GetCharacterID);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetBodyID", CharacterController_GetBodyID);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_Activate", CharacterController_Activate);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_Deactivate", CharacterController_Deactivate);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_IsActive", CharacterController_IsActive);

		/* Position and rotation */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetPosition", CharacterController_SetPosition);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetRotation", CharacterController_SetRotation);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetPosition", CharacterController_GetPosition);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetCenterOfMassPosition", CharacterController_GetCenterOfMassPosition);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetRotation", CharacterController_GetRotation);

		/* Velocities */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetLinearAndAngularVelocity", CharacterController_SetLinearAndAngularVelocity);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetLinearVelocity", CharacterController_SetLinearVelocity);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetLinearVelocity", CharacterController_GetLinearVelocity);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_AddLinearVelocity", CharacterController_AddLinearVelocity);

		/* Shape */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetShape", CharacterController_SetShape);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetShapeID", CharacterController_GetShapeID);

		/* Impulses */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_AddImpulse", CharacterController_AddImpulse);

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
