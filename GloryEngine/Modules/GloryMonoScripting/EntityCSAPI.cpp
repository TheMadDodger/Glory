#include "EntityCSAPI.h"
#include "AssemblyDomain.h"
#include "MonoSceneManager.h"
#include "MonoSceneObjectManager.h"
#include "MonoScriptObjectManager.h"
#include "CoreCSAPI.h"
#include "MathCSAPI.h"
#include "MonoAssetManager.h"

#include <GScene.h>
#include <SceneManager.h>
#include <ComponentTypes.h>
#include <Components.h>
#include <LayerManager.h>

namespace Glory
{
#pragma region Entity

	GScene* GetEntityScene(UUID sceneID)
	{
		if (sceneID == 0) return nullptr;
		GScene* pScene = Game::GetGame().GetEngine()->GetSceneManager()->GetOpenScene(sceneID);
		if (pScene == nullptr) return nullptr;
		return pScene;
	}

	GScene* GetEntityScene(MonoEntityHandle* pEntityHandle)
	{
		if (pEntityHandle->m_EntityID == 0) return nullptr;
		return GetEntityScene((UUID)pEntityHandle->m_SceneID);
	}

	template<typename T>
	static T& GetComponent(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		GScene* pScene = GetEntityScene(pEntityHandle);
		Utils::ECS::EntityView* pEntityView = pScene->GetRegistry().GetEntityView(pEntityHandle->m_EntityID);
		uint32_t hash = pEntityView->ComponentType(componentID);
		return pScene->GetRegistry().GetComponent<T>(pEntityHandle->m_EntityID);
	}

	bool Entity_IsValid(MonoEntityHandle* pMonoEntityHandle)
	{
		if (pMonoEntityHandle->m_EntityID == 0 || pMonoEntityHandle->m_SceneID == 0) return false;
		GScene* pScene = Game::GetGame().GetEngine()->GetSceneManager()->GetOpenScene((UUID)pMonoEntityHandle->m_SceneID);
		if (pScene == nullptr) return false;
		return pScene->GetEntityByEntityID(pMonoEntityHandle->m_EntityID).IsValid();
	}

	MonoObject* Entity_GetSceneObjectID(MonoEntityHandle* pMonoEntityHandle)
	{
		GScene* pScene = (GScene*)Game::GetGame().GetEngine()->GetSceneManager()->GetOpenScene(pMonoEntityHandle->m_SceneID);
		if (!pScene) return nullptr;
		MonoSceneObjectManager* pObjectManager = MonoSceneManager::GetSceneObjectManager(pScene);
		if (!pObjectManager) return nullptr;
		Entity entity = pScene->GetEntityByEntityID(pMonoEntityHandle->m_EntityID);
		if (!entity.IsValid()) return nullptr;
		return pObjectManager->GetMonoSceneObject(pScene->GetEntityUUID(entity.GetEntityID()));
	}

	MonoEntityHandle GetEntityHandle(MonoObject* pObject)
	{
		AssemblyDomain* pDomain = MonoManager::Instance()->ActiveDomain();
		MonoScriptObjectManager* pScriptObjects = MonoManager::Instance()->ActiveDomain()->ScriptObjectManager();
		const UUID uuid = pScriptObjects->GetIDForMonoScriptObject(pObject);
		const UUID sceneID = pScriptObjects->GetSceneIDForMonoScriptObject(pObject);
		if (!uuid || !sceneID) return MonoEntityHandle();
		GScene* pScene = GetEntityScene(sceneID);
		if (!pScene) return MonoEntityHandle();
		Entity entity = pScene->GetEntityByUUID(uuid);
		if (!entity.IsValid()) return MonoEntityHandle();
		Utils::ECS::EntityID entityID = entity.GetEntityID();
		return MonoEntityHandle(entityID, pScene->GetUUID());
	}

	uint64_t Component_GetComponentID(MonoEntityHandle* pEntityHandle, MonoString* pComponentName)
	{
		const std::string componentName{ mono_string_to_utf8(pComponentName) };
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return 0;
		GScene* pScene = Game::GetGame().GetEngine()->GetSceneManager()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		Utils::ECS::EntityView* pEntityView = pScene->GetRegistry().GetEntityView(pEntityHandle->m_EntityID);

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
		GScene* pScene = Game::GetGame().GetEngine()->GetSceneManager()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		const UUID uuid{};
		pScene->GetRegistry().CreateComponent(pEntityHandle->m_EntityID, componentHash, uuid);
		return uuid;
	}

	uint64_t Component_RemoveComponent(MonoEntityHandle* pEntityHandle, MonoString* pComponentName)
	{
		const std::string componentName{ mono_string_to_utf8(pComponentName) };
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return 0;
		GScene* pScene = Game::GetGame().GetEngine()->GetSceneManager()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		return pScene->GetRegistry().RemoveComponent(pEntityHandle->m_EntityID, componentHash);
	}

	void Component_RemoveComponentByID(MonoEntityHandle* pEntityHandle, uint64_t id)
	{
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return;
		GScene* pScene = Game::GetGame().GetEngine()->GetSceneManager()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return;
		Utils::ECS::EntityView* pEntityView = pScene->GetRegistry().GetEntityView(pEntityHandle->m_EntityID);
		const uint32_t hash = pEntityView->ComponentType(id);
		pScene->GetRegistry().RemoveComponent(pEntityHandle->m_EntityID, hash);
	}

	bool EntityComponent_GetActive(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return false;
		GScene* pScene = GetEntityScene(pEntityHandle);
		Utils::ECS::EntityView* pEntityView = pScene->GetRegistry().GetEntityView(pEntityHandle->m_EntityID);
		const uint32_t type = pEntityView->ComponentType(componentID);
		Utils::ECS::BaseTypeView* pTypeView = pScene->GetRegistry().GetTypeView(type);
		return pTypeView->IsActive(pEntityHandle->m_EntityID);
	}

	void EntityComponent_SetActive(MonoEntityHandle* pEntityHandle, uint64_t componentID, bool active)
	{
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return;
		GScene* pScene = GetEntityScene(pEntityHandle);
		Utils::ECS::EntityView* pEntityView = pScene->GetRegistry().GetEntityView(pEntityHandle->m_EntityID);
		const uint32_t type = pEntityView->ComponentType(componentID);
		Utils::ECS::BaseTypeView* pTypeView = pScene->GetRegistry().GetTypeView(type);
		pTypeView->SetActive(pEntityHandle->m_EntityID, active);
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

#pragma region SceneObject

	MonoEntityHandle SceneObject_GetEntityHandle(uint64_t objectID, uint64_t sceneID)
	{
		GScene* pScene = GetEntityScene(sceneID);
		if(!pScene) return MonoEntityHandle();
		Entity entity = pScene->GetEntityByUUID(objectID);
		if (!entity.IsValid()) return MonoEntityHandle();
		Utils::ECS::EntityID entityID = entity.GetEntityID();
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

		BIND("GloryEngine.Entities.EntityComponent::EntityComponent_GetActive", EntityComponent_GetActive);
		BIND("GloryEngine.Entities.EntityComponent::EntityComponent_SetActive", EntityComponent_SetActive);

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

		/* Entity Scene Object */
		BIND("GloryEngine.Entities.SceneObject::SceneObject_GetEntityHandle", SceneObject_GetEntityHandle);
	}

	MonoEntityHandle::MonoEntityHandle() : m_EntityID(0), m_SceneID(0)
	{
	}

	MonoEntityHandle::MonoEntityHandle(uint64_t entityID, uint64_t sceneID) : m_EntityID(entityID), m_SceneID(sceneID)
	{
	}

#pragma endregion
}
