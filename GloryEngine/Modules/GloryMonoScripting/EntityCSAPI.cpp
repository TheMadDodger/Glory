#include "EntityCSAPI.h"
#include "AssemblyDomain.h"
#include "CoreCSAPI.h"
#include "MathCSAPI.h"
#include "ScriptingExtender.h"
#include "MonoComponents.h"
#include "GloryMonoScipting.h"
#include "CoreLibManager.h"

#include <GScene.h>
#include <SceneManager.h>
#include <ComponentTypes.h>
#include <Components.h>
#include <AudioComponents.h>
#include <AudioModule.h>
#include <RendererModule.h>
#include <AudioSourceSystem.h>
#include <LayerManager.h>
#include <ComponentHelpers.h>

namespace Glory
{
	Engine* Entity_EngineInstance;

#pragma region Entity

	GScene* GetEntityScene(uint64_t sceneID)
	{
		if (sceneID == 0) return nullptr;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene(UUID(sceneID));
		return pScene;
	}

	template<typename T>
	static T& GetComponent(UUID sceneID, UUID objectID, uint64_t componentID)
	{
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();
		return pScene->GetRegistry().GetComponent<T>(entity.GetEntityID());
	}

	uint64_t SceneObject_GetComponentID(uint64_t sceneID, uint64_t objectID, MonoString* pComponentName)
	{
		const std::string componentName{ mono_string_to_utf8(pComponentName) };
		if (objectID == 0 || sceneID == 0) return 0;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)sceneID);
		if (pScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();

		for (auto iter = pEntityView->GetIterator(); iter != pEntityView->GetIteratorEnd(); iter++)
		{
			if (iter->second != componentHash) continue;
			return iter->first;
		}
		return 0;
	}

	uint64_t SceneObject_AddComponent(uint64_t sceneID, uint64_t objectID, MonoString* pComponentName)
	{
		const std::string componentName{ mono_string_to_utf8(pComponentName) };
		if (objectID == 0 || sceneID == 0) return 0;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)sceneID);
		if (pScene == nullptr) return 0;
		Entity entity = pScene->GetEntityByUUID(objectID);
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		const UUID uuid{};

		Utils::ECS::EntityRegistry& registry = pScene->GetRegistry();
		if (registry.HasComponent(entity.GetEntityID(), componentHash))
		{
			Entity_EngineInstance->GetDebug().LogError("Mutliple components of the same type on one entity is currently not supported");
			return 0;
		}

		void* pNewComponent = registry.CreateComponent(entity.GetEntityID(), componentHash, uuid);
		Utils::ECS::BaseTypeView* pTypeView = registry.GetTypeView(componentHash);
		pTypeView->Invoke(Utils::ECS::InvocationType::OnValidate, &registry, entity.GetEntityID(), pNewComponent);
		pTypeView->Invoke(Utils::ECS::InvocationType::Start, &registry, entity.GetEntityID(), pNewComponent);
		/* We can assume the component is active, but is the entity active? */
		if (entity.IsActive())
			pTypeView->Invoke(Utils::ECS::InvocationType::OnEnable, &registry, entity.GetEntityID(), pNewComponent);

		return uuid;
	}
	
	uint64_t SceneObject_AddScriptComponent(uint64_t sceneID, uint64_t objectID, int typeIndex)
	{
		if (objectID == 0 || sceneID == 0) return 0;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)sceneID);
		if (pScene == nullptr) return 0;
		Entity entity = pScene->GetEntityByUUID(objectID);
		const uint32_t typeHash = MonoManager::Instance()->GetCoreLibManager()->ScriptManager().TypeHash((size_t)typeIndex);
		const UUID uuid{};
		Utils::ECS::EntityRegistry& registry = pScene->GetRegistry();
		if (registry.HasComponent<MonoScriptComponent>(entity.GetEntityID()))
		{
			Entity_EngineInstance->GetDebug().LogError("Mutliple script components on one entity is currently not supported");
			return 0;
		}

		MonoScriptComponent& comp = registry.AddComponent<MonoScriptComponent>(entity.GetEntityID(), uuid, typeHash);
		Utils::ECS::TypeView<MonoScriptComponent>* pTypeView = registry.GetTypeView<MonoScriptComponent>();
		pTypeView->Invoke(Utils::ECS::InvocationType::OnValidate, &registry, entity.GetEntityID(), &comp);
		pTypeView->Invoke(Utils::ECS::InvocationType::Start, &registry, entity.GetEntityID(), &comp);
		/* If the scene is starting the enable callback will be called for us */
		/* We can assume the component is active, but is the entity active? */
		if (entity.IsActive() && !pScene->IsStarting())
			pTypeView->Invoke(Utils::ECS::InvocationType::OnEnable, &registry, entity.GetEntityID(), &comp);
		return uuid;
	}

	uint64_t SceneObject_RemoveComponent(uint64_t sceneID, uint64_t objectID, MonoString* pComponentName)
	{
		const std::string componentName{ mono_string_to_utf8(pComponentName) };
		if (objectID == 0 || sceneID == 0) return 0;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)sceneID);
		if (pScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		Entity entity = pScene->GetEntityByUUID(objectID);

		Utils::ECS::EntityRegistry& registry = pScene->GetRegistry();
		Utils::ECS::BaseTypeView* pTypeView = registry.GetTypeView(componentHash);
		const uint32_t index = pTypeView->GetComponentIndex(entity.GetEntityID());
		return Components::Destroy(entity, pTypeView, index);
	}

	void SceneObject_RemoveComponentByID(uint64_t sceneID, uint64_t objectID, uint64_t id)
	{
		if (objectID == 0 || sceneID == 0) return;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)sceneID);
		if (pScene == nullptr) return;
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = pScene->GetRegistry().GetEntityView(entity.GetEntityID());
		const uint32_t hash = pEntityView->ComponentType(id);

		Utils::ECS::EntityRegistry& registry = pScene->GetRegistry();
		Utils::ECS::BaseTypeView* pTypeView = registry.GetTypeView(hash);
		const size_t index = pTypeView->GetComponentIndex(entity.GetEntityID());
		Components::Destroy(entity, pTypeView, index);
	}

	bool EntityComponent_GetActive(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		if (objectID == 0 || sceneID == 0) return false;
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();
		const uint32_t type = pEntityView->ComponentType(componentID);
		Utils::ECS::BaseTypeView* pTypeView = pScene->GetRegistry().GetTypeView(type);
		return pTypeView->IsActive(entity.GetEntityID());
	}

	void EntityComponent_SetActive(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool active)
	{
		if (objectID == 0 || sceneID == 0) return;
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();
		const uint32_t type = pEntityView->ComponentType(componentID);
		Utils::ECS::BaseTypeView* pTypeView = pScene->GetRegistry().GetTypeView(type);
		const size_t componentIndex = pTypeView->GetComponentIndex(entity.GetEntityID());

		if (active)
			Components::Activate(entity, pTypeView, componentIndex);
		else
			Components::Deactivate(entity, pTypeView, componentIndex);
	}

	bool EntityBehaviour_GetActive(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		if (objectID == 0 || sceneID == 0) return false;
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::TypeView<MonoScriptComponent>* pTypeView = pScene->GetRegistry().GetTypeView<MonoScriptComponent>();
		return pTypeView->IsActive(entity.GetEntityID());
	}

	void EntityBehaviour_SetActive(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool active)
	{
		if (objectID == 0 || sceneID == 0) return;
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::TypeView<MonoScriptComponent>* pTypeView = pScene->GetRegistry().GetTypeView<MonoScriptComponent>();
		const size_t componentIndex = pTypeView->GetComponentIndex(entity.GetEntityID());

		if (active)
			Components::Activate(entity, pTypeView, componentIndex);
		else
			Components::Deactivate(entity, pTypeView, componentIndex);
	}

#pragma endregion

#pragma region Transform

	glm::vec3 Transform_GetLocalPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		return transform.Position;
	}

	void Transform_SetLocalPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID, glm::vec3* position)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		transform.Position = *position;
		Entity entity = GetEntityScene(sceneID)->GetEntityByUUID(objectID);
		entity.SetDirty();
	}

	QuatWrapper Transform_GetLocalRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		return transform.Rotation;
	}

	void Transform_SetLocalRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID, glm::quat* rotation)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		transform.Rotation = *rotation;

		Entity entity = GetEntityScene(sceneID)->GetEntityByUUID(objectID);
		entity.SetDirty();
	}

	glm::vec3 Transform_GetLocalRotationEuler(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		return glm::eulerAngles(transform.Rotation);
	}

	void Transform_SetLocalRotationEuler(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper* rotation)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		transform.Rotation = glm::quat(ToGLMVec3(*rotation));

		Entity entity = GetEntityScene(sceneID)->GetEntityByUUID(objectID);
		entity.SetDirty();
	}

	glm::vec3 Transform_GetLocalScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		return transform.Scale;
	}

	void Transform_SetLocalScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID, glm::vec3* scale)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		transform.Scale = *scale;

		Entity entity = GetEntityScene(sceneID)->GetEntityByUUID(objectID);
		entity.SetDirty();
	}

	Vec3Wrapper Transform_GetWorldPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		glm::vec3 translation;
		glm::decompose(transform.MatTransform, glm::vec3(), glm::quat(), translation, glm::vec3(), glm::vec4());
		return translation;
	}

	void Transform_SetWorldPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper* position)
	{
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();

		Transform& transform = entity.GetComponent<Transform>();
		const Utils::ECS::EntityID parent = entity.Parent();

		glm::mat4 parentTransform = glm::identity<glm::mat4>();

		if (parent)
			parentTransform = pScene->GetRegistry().GetComponent<Transform>(parent).MatTransform;

		const glm::vec4 worldPosition = glm::vec4(ToGLMVec3(*position), 1.0f);
		transform.Position = glm::inverse(parentTransform)*worldPosition;
		entity.SetDirty(true);
	}

	QuatWrapper Transform_GetWorldRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		glm::mat3 mat = transform.MatTransform;
		mat[0] = glm::normalize(mat[0]);
		mat[1] = glm::normalize(mat[1]);
		mat[2] = glm::normalize(mat[2]);
		return glm::quat_cast(mat);
	}

	void Transform_SetWorldRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID, QuatWrapper* rotation)
	{
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();

		Transform& transform = entity.GetComponent<Transform>();
		const Utils::ECS::EntityID parent = entity.Parent();

		glm::mat4 parentTransform = glm::identity<glm::mat4>();

		if (parent)
			parentTransform = pScene->GetRegistry().GetComponent<Transform>(parent).MatTransform;

		const glm::quat worldRotation = ToGLMQuat(*rotation);
		glm::mat3 transformRotation = glm::inverse(parentTransform)*glm::mat4_cast(worldRotation);
		transformRotation[0] = glm::normalize(transformRotation[0]);
		transformRotation[1] = glm::normalize(transformRotation[1]);
		transformRotation[2] = glm::normalize(transformRotation[2]);
		transform.Rotation = glm::inverse(glm::quat_cast(transformRotation));
		entity.SetDirty(true);
	}

	Vec3Wrapper Transform_GetWorldRotationEuler(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		glm::mat3 mat = transform.MatTransform;
		mat[0] = glm::normalize(mat[0]);
		mat[1] = glm::normalize(mat[1]);
		mat[2] = glm::normalize(mat[2]);
		const glm::quat rotation = glm::quat_cast(mat);
		return glm::eulerAngles(rotation);
	}

	void Transform_SetWorldRotationEuler(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper* rotation)
	{
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();

		Transform& transform = entity.GetComponent<Transform>();
		const Utils::ECS::EntityID parent = entity.Parent();

		glm::mat4 parentTransform = glm::identity<glm::mat4>();

		if (parent)
			parentTransform = pScene->GetRegistry().GetComponent<Transform>(parent).MatTransform;

		const glm::quat worldRotation = glm::quat(ToGLMVec3(*rotation));
		glm::mat3 transformRotation = glm::inverse(parentTransform)*glm::mat4_cast(worldRotation);
		transformRotation[0] = glm::normalize(transformRotation[0]);
		transformRotation[1] = glm::normalize(transformRotation[1]);
		transformRotation[2] = glm::normalize(transformRotation[2]);
		transform.Rotation = glm::inverse(glm::quat_cast(transformRotation));
		entity.SetDirty(true);
	}

	Vec3Wrapper Transform_GetWorldScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		glm::vec3 scale;
		glm::decompose(transform.MatTransform, scale, glm::quat(), glm::vec3(), glm::vec3(), glm::vec4());
		return scale;
	}

	void Transform_SetWorldScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper* scale)
	{
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();

		Transform& transform = entity.GetComponent<Transform>();
		const Utils::ECS::EntityID parent = entity.Parent();

		glm::mat4 parentTransform = glm::identity<glm::mat4>();

		if (parent)
			parentTransform = pScene->GetRegistry().GetComponent<Transform>(parent).MatTransform;

		const glm::vec4 worldScale = glm::vec4(ToGLMVec3(*scale), 0.0f);
		transform.Scale = glm::inverse(parentTransform)*worldScale;
		entity.SetDirty(true);
	}

	glm::vec3 Transform_GetForward(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		return glm::vec3(transform.MatTransform[2][0], transform.MatTransform[2][1], transform.MatTransform[2][2]);
	}

	glm::vec3 Transform_GetUp(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		return glm::vec3(transform.MatTransform[1][0], transform.MatTransform[1][1], transform.MatTransform[1][2]);
	}

	void Transform_SetForward(uint64_t sceneID, uint64_t objectID, uint64_t componentID, glm::vec3* forward)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		transform.Rotation = glm::conjugate(glm::quatLookAt(*forward, { 0.0f, 1.0f, 0.0f }));

		Entity entity = GetEntityScene(sceneID)->GetEntityByUUID(objectID);
		entity.SetDirty();
	}

	glm::vec3 Transform_GetRight(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		return glm::vec3(transform.MatTransform[0][0], transform.MatTransform[0][1], transform.MatTransform[0][2]);
	}

	Mat4Wrapper Transform_GetWorld(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(sceneID, objectID, componentID);
		return ToMat4Wrapper(transform.MatTransform);
	}

#pragma endregion

#pragma region MeshRenderer

	uint64_t MeshRenderer_GetMaterial(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(sceneID, objectID, componentID);
		return meshRenderer.m_Material.AssetUUID();
	}

	void MeshRenderer_SetMaterial(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t materialID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(sceneID, objectID, componentID);
		meshRenderer.m_Material = UUID(materialID);
	}

	uint64_t MeshRenderer_GetMesh(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(sceneID, objectID, componentID);
		return meshRenderer.m_Mesh.AssetUUID();
	}

	void MeshRenderer_SetMesh(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t meshID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(sceneID, objectID, componentID);
		meshRenderer.m_Mesh = UUID(meshID);
	}

#pragma endregion

#pragma region ModelRenderer

	uint64_t ModelRenderer_GetMaterial(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(sceneID, objectID, componentID);
		return meshRenderer.m_Materials.size() > 0 ? meshRenderer.m_Materials[0].m_MaterialReference.AssetUUID() : 0;
	}

	void ModelRenderer_SetMaterial(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t materialID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(sceneID, objectID, componentID);
		if (meshRenderer.m_Materials.size() <= 0) meshRenderer.m_Materials.push_back(UUID(materialID));
		else meshRenderer.m_Materials[0] = UUID(materialID);
	}

	size_t ModelRenderer_GetMaterialCount(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(sceneID, objectID, componentID);
		return meshRenderer.m_Materials.size();
	}

	uint64_t ModelRenderer_GetMaterialAt(uint64_t sceneID, uint64_t objectID, uint64_t componentID, size_t index)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(sceneID, objectID, componentID);
		if (meshRenderer.m_Materials.size() <= index) return 0;
		return meshRenderer.m_Materials[index].m_MaterialReference.AssetUUID();
	}

	void ModelRenderer_AddMaterial(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t materialID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(sceneID, objectID, componentID);
		meshRenderer.m_Materials.push_back(UUID(materialID));
	}

	void ModelRenderer_SetMaterialAt(uint64_t sceneID, uint64_t objectID, uint64_t componentID, size_t index, uint64_t materialID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(sceneID, objectID, componentID);
		if (meshRenderer.m_Materials.size() <= index) return;
		meshRenderer.m_Materials[index] = UUID(materialID);
	}

	void ModelRenderer_ClearMaterials(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(sceneID, objectID, componentID);
		meshRenderer.m_Materials.clear();
	}

	uint64_t ModelRenderer_GetModel(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(sceneID, objectID, componentID);
		return meshRenderer.m_Model.AssetUUID();
	}

	void ModelRenderer_SetModel(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t modelID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(sceneID, objectID, componentID);
		meshRenderer.m_Model = UUID(modelID);
	}

#pragma endregion

#pragma region CameraComponent

	float CameraComponent_GetHalfFOV(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		return cameraComp.m_HalfFOV;
	}

	void CameraComponent_SetHalfFOV(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float halfFov)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		cameraComp.m_HalfFOV = halfFov;
	}

	float CameraComponent_GetNear(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		return cameraComp.m_Near;
	}

	void CameraComponent_SetNear(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float near)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		cameraComp.m_Near = near;
	}

	float CameraComponent_GetFar(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		return cameraComp.m_Far;
	}

	void CameraComponent_SetFar(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float far)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		cameraComp.m_Far = far;
	}

	int CameraComponent_GetDisplayIndex(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		return cameraComp.m_DisplayIndex;
	}

	void CameraComponent_SetDisplayIndex(uint64_t sceneID, uint64_t objectID, uint64_t componentID, int displayIndex)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		cameraComp.m_DisplayIndex = displayIndex;
	}

	int CameraComponent_GetPriority(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		return cameraComp.m_Priority;
	}

	void CameraComponent_SetPriority(uint64_t sceneID, uint64_t objectID, uint64_t componentID, int priority)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		cameraComp.m_Priority = priority;
	}

	LayerMask CameraComponent_GetLayerMask(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		return cameraComp.m_LayerMask;
	}

	void CameraComponent_SetLayerMask(uint64_t sceneID, uint64_t objectID, uint64_t componentID, LayerMask* pLayerMask)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		cameraComp.m_LayerMask.m_Mask = pLayerMask->m_Mask;
	}

	glm::vec4 CameraComponent_GetClearColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		return cameraComp.m_ClearColor;
	}

	void CameraComponent_SetClearColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID, glm::vec4* clearCol)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		cameraComp.m_ClearColor = *clearCol;
	}

	uint64_t CameraComponent_GetCameraID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		return cameraComp.m_Camera.GetUUID();
	}
	
	void CameraComponent_PrepareNextPick(uint64_t sceneID, uint64_t objectID, uint64_t componentID, glm::vec2* position)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		RendererModule* pRenderer = Entity_EngineInstance->GetMainModule<RendererModule>();
		if (!cameraComp.m_Camera.GetUUID()) return;
		pRenderer->Submit(glm::ivec2(*position), cameraComp.m_Camera.GetUUID());
	}

	struct PickResultWrapper
	{
		uint64_t CameraID;
		uint64_t ObjectID;
		Vec3Wrapper Position;
		Vec3Wrapper Normal;
	};
	
	PickResultWrapper CameraComponent_GetPickResult(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		RendererModule* pRenderer = Entity_EngineInstance->GetMainModule<RendererModule>();
		PickResultWrapper result{ 0, 0, Vec3Wrapper{{}}, Vec3Wrapper{{}} };

		pRenderer->GetPickResult(cameraComp.m_Camera.GetUUID(), [&result](const PickResult& pickResult) {
			uint64_t pickedObjectID = 0;

			GScene* pScene = (GScene*)Entity_EngineInstance->GetSceneManager()->GetOpenScene(pickResult.m_Object.SceneUUID());
			if (pScene)
				pickedObjectID = pickResult.m_Object.ObjectUUID();

			result = PickResultWrapper{ pickResult.m_CameraID, pickedObjectID,
				ToVec3Wrapper(pickResult.m_WorldPosition), ToVec3Wrapper(pickResult.m_Normal) };
		});
		return result;
	}
	
	Vec3Wrapper CameraComponent_GetResolution(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(sceneID, objectID, componentID);
		const glm::uvec2& resolution = cameraComp.m_Camera.GetResolution();
		return Vec3Wrapper(resolution.x, resolution.y, 0.0f);
	}

#pragma endregion

#pragma region Layer Component

	LayerWrapper LayerComponent_GetLayer(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		LayerComponent& layerComp = GetComponent<LayerComponent>(sceneID, objectID, componentID);
		return LayerWrapper(layerComp.m_Layer.Layer(&Entity_EngineInstance->GetLayerManager()));
	}

	void LayerComponent_SetLayer(uint64_t sceneID, uint64_t objectID, uint64_t componentID, LayerWrapper* layer)
	{
		LayerComponent& layerComp = GetComponent<LayerComponent>(sceneID, objectID, componentID);
		const Layer* pLayer = Entity_EngineInstance->GetLayerManager().GetLayerByName(mono_string_to_utf8(layer->Name));
		layerComp.m_Layer = pLayer ? Entity_EngineInstance->GetLayerManager().GetLayerIndex(pLayer) + 1 : 0;
	}

#pragma endregion

#pragma region Light Component

	glm::vec4 LightComponent_GetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(sceneID, objectID, componentID);
		return lightComp.m_Color;
	}

	void LightComponent_SetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID, glm::vec4* color)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(sceneID, objectID, componentID);
		lightComp.m_Color = *color;
	}

	float LightComponent_GetIntensity(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(sceneID, objectID, componentID);
		return lightComp.m_Intensity;
	}

	void LightComponent_SetIntensity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float intensity)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(sceneID, objectID, componentID);
		lightComp.m_Intensity = intensity;
	}

	float LightComponent_GetRange(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(sceneID, objectID, componentID);
		return lightComp.m_Range;
	}

	void LightComponent_SetRange(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float range)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(sceneID, objectID, componentID);
		lightComp.m_Range = range;
	}

#pragma endregion

#pragma region AudioSource

	uint64_t AudioSource_GetAudio(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		return source.m_Audio.AssetUUID();
	}

	void AudioSource_SetAudio(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t audioID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		source.m_Audio = UUID(audioID);
	}

	bool AudioSource_GetAsMusic(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		return source.m_AsMusic;
	}

	void AudioSource_SetAsMusic(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool asMusic)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		source.m_AsMusic = asMusic;
	}

	uint32_t AudioSource_GetLoops(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		return source.m_Loops;
	}

	void AudioSource_SetLoops(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint32_t loops)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		source.m_Loops = loops;
	}

	bool AudioSource_GetEnable3D(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		return source.m_Enable3D;
	}

	void AudioSource_SetEnable3D(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool allow)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		source.m_Enable3D = allow;
	}

	bool AudioSource_GetAutoPlay(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		return source.m_AutoPlay;
	}

	void AudioSource_SetAutoPlay(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool autoPlay)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		source.m_AutoPlay = autoPlay;
	}

	bool AudioSource_GetPlaying(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		if (source.m_CurrentChannel == -1) return false;
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_GetPlaying > No audio module was loaded to play audio.");
			return false;
		}
		if (source.m_AsMusic)
			return pAudioModule->IsMusicPlaying();
		return pAudioModule->IsPlaying(source.m_CurrentChannel);
	}

	bool AudioSource_GetPaused(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		if (source.m_CurrentChannel == -1) return false;
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_GetPaused > No audio module was loaded to play audio.");
			return false;
		}

		if (source.m_AsMusic)
			return pAudioModule->IsMusicPaused();
		return pAudioModule->IsPaused(source.m_CurrentChannel);
	}

	void AudioSource_SetPaused(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool pause)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		if (source.m_CurrentChannel == -1) return;
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_SetPaused > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		if (pause)
			AudioSourceSystem::Pause(&pScene->GetRegistry(), entity.GetEntityID(), source);
		else
			AudioSourceSystem::Resume(&pScene->GetRegistry(), entity.GetEntityID(), source);
	}

	float AudioSource_GetVolume(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		return source.m_Volume;
	}

	void AudioSource_SetVolume(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float volume)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		source.m_Volume = volume;
		if (source.m_CurrentChannel == -1) return;
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_SetVolume > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		AudioSourceSystem::UpdateVolume(&pScene->GetRegistry(), entity.GetEntityID(), source);
	}

	void AudioSource_Play(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_Play > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		AudioSourceSystem::Play(&pScene->GetRegistry(), entity.GetEntityID(), source);
	}

	void AudioSource_Stop(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_Stop > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		AudioSourceSystem::Stop(&pScene->GetRegistry(), entity.GetEntityID(), source);
	}

	void AudioSource_Pause(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_Pause > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		AudioSourceSystem::Stop(&pScene->GetRegistry(), entity.GetEntityID(), source);
	}

	void AudioSource_Resume(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_Resume > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		AudioSourceSystem::Resume(&pScene->GetRegistry(), entity.GetEntityID(), source);
	}

	SpatializationSettings* AudioSource_GetSpatializationSettings(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		return &source.m_Spatialization;
	}

	AudioSourceSimulationSettings* AudioSource_GetSimulationSettings(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(sceneID, objectID, componentID);
		return &source.m_Simulation;
	}

#pragma endregion

#pragma region Audio listener

	bool AudioListener_GetEnabled(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioListener& listener = GetComponent<AudioListener>(sceneID, objectID, componentID);
		return listener.m_Enable;
	}

	void AudioListener_SetEnabled(uint64_t sceneID, uint64_t objectID, uint64_t componentID, bool value)
	{
		AudioListener& listener = GetComponent<AudioListener>(sceneID, objectID, componentID);
		listener.m_Enable = value;
	}

	AudioSimulationSettings* AudioListener_GetSimulationSettings(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		AudioListener& listener = GetComponent<AudioListener>(sceneID, objectID, componentID);
		return &listener.m_Simulation;
	}

#pragma endregion

#pragma region Text Components

	uint64_t TextComponent_GetFont(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		return text.m_Font.AssetUUID();
	}

	void TextComponent_SetFont(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t fontID)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		text.m_Font.SetUUID(fontID);
		text.m_Dirty = true;
	}

	MonoString* TextComponent_GetText(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		return mono_string_new(mono_domain_get(), text.m_Text.c_str());
	}

	void TextComponent_SetText(uint64_t sceneID, uint64_t objectID, uint64_t componentID, MonoString* str)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		text.m_Text = mono_string_to_utf8(str);
		text.m_Dirty = true;
	}

	float TextComponent_GetScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		return text.m_Scale;
	}

	void TextComponent_SetScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float scale)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		text.m_Scale = uint32_t(scale);
		text.m_Dirty = true;
	}

	glm::vec4 TextComponent_GetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		return text.m_Color;
	}

	void TextComponent_SetColor(uint64_t sceneID, uint64_t objectID, uint64_t componentID, glm::vec4* clearCol)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		text.m_Color = *clearCol;
		text.m_Dirty = true;
	}

	void TextComponent_SetAlignment(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Alignment alignment)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		text.m_Alignment = alignment;
		text.m_Dirty = true;
	}

	Alignment TextComponent_GetAlignment(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		return text.m_Alignment;
	}

	void TextComponent_SetWrapWidth(uint64_t sceneID, uint64_t objectID, uint64_t componentID, float wrap)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		text.m_WrapWidth = wrap;
		text.m_Dirty = true;
	}

	float TextComponent_GetWrapWidth(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		TextComponent& text = GetComponent<TextComponent>(sceneID, objectID, componentID);
		return text.m_WrapWidth;
	}

#pragma endregion

#pragma region Binding

	void EntityCSAPI::GetInternallCalls(std::vector<InternalCall>& internalCalls)
	{
		/* Entity */
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_GetComponentID", SceneObject_GetComponentID);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_AddComponent", SceneObject_AddComponent);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_AddScriptComponent", SceneObject_AddScriptComponent);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_RemoveComponent", SceneObject_RemoveComponent);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_RemoveComponentByID", SceneObject_RemoveComponentByID);

		BIND("GloryEngine.Entities.EntityComponent::EntityComponent_GetActive", EntityComponent_GetActive);
		BIND("GloryEngine.Entities.EntityComponent::EntityComponent_SetActive", EntityComponent_SetActive);
		
		BIND("GloryEngine.Entities.EntityBehaviour::EntityBehaviour_GetActive", EntityBehaviour_GetActive);
		BIND("GloryEngine.Entities.EntityBehaviour::EntityBehaviour_SetActive", EntityBehaviour_SetActive);

		/* Transform */
		BIND("GloryEngine.Entities.Transform::Transform_GetLocalPosition", Transform_GetLocalPosition);
		BIND("GloryEngine.Entities.Transform::Transform_SetLocalPosition", Transform_SetLocalPosition);

		BIND("GloryEngine.Entities.Transform::Transform_GetLocalRotation", Transform_GetLocalRotation);
		BIND("GloryEngine.Entities.Transform::Transform_SetLocalRotation", Transform_SetLocalRotation);

		BIND("GloryEngine.Entities.Transform::Transform_GetLocalRotationEuler", Transform_GetLocalRotationEuler);
		BIND("GloryEngine.Entities.Transform::Transform_SetLocalRotationEuler", Transform_SetLocalRotationEuler);

		BIND("GloryEngine.Entities.Transform::Transform_GetLocalScale", Transform_GetLocalScale);
		BIND("GloryEngine.Entities.Transform::Transform_SetLocalScale", Transform_SetLocalScale);

		BIND("GloryEngine.Entities.Transform::Transform_GetWorldPosition", Transform_GetWorldPosition);
		BIND("GloryEngine.Entities.Transform::Transform_SetWorldPosition", Transform_SetWorldPosition);

		BIND("GloryEngine.Entities.Transform::Transform_GetWorldRotation", Transform_GetWorldRotation);
		BIND("GloryEngine.Entities.Transform::Transform_SetWorldRotation", Transform_SetWorldRotation);

		BIND("GloryEngine.Entities.Transform::Transform_GetWorldRotationEuler", Transform_GetWorldRotationEuler);
		BIND("GloryEngine.Entities.Transform::Transform_SetWorldRotationEuler", Transform_SetWorldRotationEuler);

		BIND("GloryEngine.Entities.Transform::Transform_GetWorldScale", Transform_GetWorldScale);
		BIND("GloryEngine.Entities.Transform::Transform_SetWorldScale", Transform_SetWorldScale);

		BIND("GloryEngine.Entities.Transform::Transform_GetForward", Transform_GetForward);
		BIND("GloryEngine.Entities.Transform::Transform_SetForward", Transform_SetForward);
		BIND("GloryEngine.Entities.Transform::Transform_GetRight", Transform_GetRight);
		BIND("GloryEngine.Entities.Transform::Transform_GetUp", Transform_GetUp);
		BIND("GloryEngine.Entities.Transform::Transform_GetWorld", Transform_GetWorld);

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

		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_PrepareNextPick", CameraComponent_PrepareNextPick);
		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetPickResult", CameraComponent_GetPickResult);

		BIND("GloryEngine.Entities.CameraComponent::CameraComponent_GetResolution", CameraComponent_GetResolution);

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

		/* AudioSource */
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetAudio", AudioSource_GetAudio);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_SetAudio", AudioSource_SetAudio);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetAsMusic", AudioSource_GetAsMusic);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_SetAsMusic", AudioSource_SetAsMusic);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetLoops", AudioSource_GetLoops);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_SetLoops", AudioSource_SetLoops);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetEnable3D", AudioSource_GetEnable3D);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_SetEnable3D", AudioSource_SetEnable3D);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetAutoPlay", AudioSource_GetAutoPlay);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_SetAutoPlay", AudioSource_SetAutoPlay);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetPlaying", AudioSource_GetPlaying);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetPaused", AudioSource_GetPaused);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_SetPaused", AudioSource_SetPaused);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetVolume", AudioSource_GetVolume);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_SetVolume", AudioSource_SetVolume);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetSpatializationSettings", AudioSource_GetSpatializationSettings);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_GetSimulationSettings", AudioSource_GetSimulationSettings);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_Play", AudioSource_Play);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_Stop", AudioSource_Stop);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_Pause", AudioSource_Pause);
		BIND("GloryEngine.Entities.AudioSource::AudioSource_Resume", AudioSource_Resume);

		/* AudioListener */
		BIND("GloryEngine.Entities.AudioSource::AudioListener_GetEnabled", AudioListener_GetEnabled);
		BIND("GloryEngine.Entities.AudioSource::AudioListener_SetEnabled", AudioListener_SetEnabled);
		BIND("GloryEngine.Entities.AudioSource::AudioListener_GetSimulationSettings", AudioListener_GetSimulationSettings);

		/* Text */
		BIND("GloryEngine.Entities.TextComponent::TextComponent_GetFont", TextComponent_GetFont);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_SetFont", TextComponent_SetFont);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_GetText", TextComponent_GetText);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_SetText", TextComponent_SetText);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_GetScale", TextComponent_GetScale);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_SetScale", TextComponent_SetScale);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_GetColor", TextComponent_GetColor);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_SetColor", TextComponent_SetColor);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_SetAlignment", TextComponent_SetAlignment);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_GetAlignment", TextComponent_GetAlignment);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_SetWrapWidth", TextComponent_SetWrapWidth);
		BIND("GloryEngine.Entities.TextComponent::TextComponent_GetWrapWidth", TextComponent_GetWrapWidth);
	}

	void EntityCSAPI::SetEngine(Engine* pEngine)
	{
		Entity_EngineInstance = pEngine;
	}

#pragma endregion
}
