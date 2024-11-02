#include "EntityCSAPI.h"
#include "AssemblyDomain.h"
#include "MonoScriptObjectManager.h"
#include "CoreCSAPI.h"
#include "MathCSAPI.h"
#include "ScriptingExtender.h"
#include "MonoComponents.h"

#include <GScene.h>
#include <SceneManager.h>
#include <ComponentTypes.h>
#include <Components.h>
#include <AudioComponents.h>
#include <AudioModule.h>
#include <RendererModule.h>
#include <AudioSourceSystem.h>
#include <LayerManager.h>

namespace Glory
{
	Engine* Entity_EngineInstance;

#pragma region Entity

	GScene* GetEntityScene(uint64_t sceneID)
	{
		if (sceneID == 0) return nullptr;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene(UUID(sceneID));
		if (pScene == nullptr) return nullptr;
		return pScene;
	}

	GScene* GetEntityScene(MonoEntityHandle* pEntityHandle)
	{
		if (pEntityHandle->m_EntityID == 0) return nullptr;
		return GetEntityScene(pEntityHandle->m_SceneID);
	}

	template<typename T>
	static T& GetComponent(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		GScene* pScene = GetEntityScene(pEntityHandle);
		Utils::ECS::EntityView* pEntityView = pScene->GetRegistry().GetEntityView(pEntityHandle->m_EntityID);
		uint32_t hash = pEntityView->ComponentType(componentID);
		return pScene->GetRegistry().GetComponent<T>(pEntityHandle->m_EntityID);
	}

	bool Entity_IsValid(MonoEntityHandle* pMonoEntityHandle)
	{
		if (pMonoEntityHandle->m_EntityID == 0 || pMonoEntityHandle->m_SceneID == 0) return false;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)pMonoEntityHandle->m_SceneID);
		if (pScene == nullptr) return false;
		return pScene->GetEntityByEntityID(pMonoEntityHandle->m_EntityID).IsValid();
	}

	uint64_t Entity_GetSceneObjectID(MonoEntityHandle* pMonoEntityHandle)
	{
		GScene* pScene = (GScene*)Entity_EngineInstance->GetSceneManager()->GetOpenScene(UUID(pMonoEntityHandle->m_SceneID));
		if (!pScene) return 0;
		Entity entity = pScene->GetEntityByEntityID(pMonoEntityHandle->m_EntityID);
		if (!entity.IsValid()) return 0;
		return pScene->GetEntityUUID(entity.GetEntityID());
	}

	MonoEntityHandle GetEntityHandle(MonoObject* pObject)
	{
		AssemblyDomain* pDomain = MonoManager::Instance()->ActiveDomain();
		MonoScriptObjectManager* pScriptObjects = MonoManager::Instance()->ActiveDomain()->ScriptObjectManager();
		const uint64_t uuid = pScriptObjects->GetIDForMonoScriptObject(pObject);
		const uint64_t sceneID = pScriptObjects->GetSceneIDForMonoScriptObject(pObject);
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
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
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
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		const uint64_t uuid{};
		pScene->GetRegistry().CreateComponent(pEntityHandle->m_EntityID, componentHash, uuid);
		return uuid;
	}

	uint64_t Component_RemoveComponent(MonoEntityHandle* pEntityHandle, MonoString* pComponentName)
	{
		const std::string componentName{ mono_string_to_utf8(pComponentName) };
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return 0;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
		if (pScene == nullptr) return 0;
		const uint32_t componentHash = Glory::ComponentTypes::GetComponentHash(componentName);
		return pScene->GetRegistry().RemoveComponent(pEntityHandle->m_EntityID, componentHash);
	}

	void Component_RemoveComponentByID(MonoEntityHandle* pEntityHandle, uint64_t id)
	{
		if (pEntityHandle->m_EntityID == 0 || pEntityHandle->m_SceneID == 0) return;
		GScene* pScene = Entity_EngineInstance->GetSceneManager()->GetOpenScene((UUID)pEntityHandle->m_SceneID);
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

	glm::vec3 Transform_GetLocalPosition(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return transform.Position;
	}

	void Transform_SetLocalPosition(MonoEntityHandle* pEntityHandle, uint64_t componentID, glm::vec3* position)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Position = *position;

		GetEntityScene(pEntityHandle)->GetRegistry().SetEntityDirty(pEntityHandle->m_EntityID);
	}

	QuatWrapper Transform_GetLocalRotation(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return transform.Rotation;
	}

	void Transform_SetLocalRotation(MonoEntityHandle* pEntityHandle, uint64_t componentID, glm::quat* rotation)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Rotation = *rotation;

		GetEntityScene(pEntityHandle)->GetRegistry().SetEntityDirty(pEntityHandle->m_EntityID);
	}

	glm::vec3 Transform_GetLocalRotationEuler(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return glm::eulerAngles(transform.Rotation);
	}

	void Transform_SetLocalRotationEuler(MonoEntityHandle* pEntityHandle, uint64_t componentID, Vec3Wrapper* rotation)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Rotation = glm::quat(ToGLMVec3(*rotation));

		GetEntityScene(pEntityHandle)->GetRegistry().SetEntityDirty(pEntityHandle->m_EntityID);
	}

	glm::vec3 Transform_GetLocalScale(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return transform.Scale;
	}

	void Transform_SetLocalScale(MonoEntityHandle* pEntityHandle, uint64_t componentID, glm::vec3* scale)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Scale = *scale;

		GetEntityScene(pEntityHandle)->GetRegistry().SetEntityDirty(pEntityHandle->m_EntityID);
	}

	glm::vec3 Transform_GetForward(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return glm::vec3(transform.MatTransform[2][0], transform.MatTransform[2][1], transform.MatTransform[2][2]);
	}

	glm::vec3 Transform_GetUp(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return glm::vec3(transform.MatTransform[1][0], transform.MatTransform[1][1], transform.MatTransform[1][2]);
	}

	void Transform_SetForward(MonoEntityHandle* pEntityHandle, uint64_t componentID, glm::vec3* forward)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		transform.Rotation = glm::conjugate(glm::quatLookAt(*forward, { 0.0f, 1.0f, 0.0f }));

		GetEntityScene(pEntityHandle)->GetRegistry().SetEntityDirty(pEntityHandle->m_EntityID);
	}

	glm::vec3 Transform_GetRight(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return glm::vec3(transform.MatTransform[0][0], transform.MatTransform[0][1], transform.MatTransform[0][2]);
	}

	Mat4Wrapper Transform_GetWorld(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		Transform& transform = GetComponent<Transform>(pEntityHandle, componentID);
		return ToMat4Wrapper(transform.MatTransform);
	}

#pragma endregion

#pragma region MeshRenderer

	uint64_t MeshRenderer_GetMaterial(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(pEntityHandle, componentID);
		return meshRenderer.m_Material.AssetUUID();
	}

	void MeshRenderer_SetMaterial(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint64_t materialID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Material = UUID(materialID);
	}

	uint64_t MeshRenderer_GetMesh(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(pEntityHandle, componentID);
		return meshRenderer.m_Mesh.AssetUUID();
	}

	void MeshRenderer_SetMesh(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint64_t meshID)
	{
		MeshRenderer& meshRenderer = GetComponent<MeshRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Mesh = UUID(meshID);
	}

#pragma endregion

#pragma region ModelRenderer

	uint64_t ModelRenderer_GetMaterial(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		return meshRenderer.m_Materials.size() > 0 ? meshRenderer.m_Materials[0].m_MaterialReference.AssetUUID() : 0;
	}

	void ModelRenderer_SetMaterial(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint64_t materialID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		if (meshRenderer.m_Materials.size() <= 0) meshRenderer.m_Materials.push_back(UUID(materialID));
		else meshRenderer.m_Materials[0] = UUID(materialID);
	}

	size_t ModelRenderer_GetMaterialCount(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		return meshRenderer.m_Materials.size();
	}

	uint64_t ModelRenderer_GetMaterialAt(MonoEntityHandle* pEntityHandle, uint64_t componentID, size_t index)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		if (meshRenderer.m_Materials.size() <= index) return 0;
		return meshRenderer.m_Materials[index].m_MaterialReference.AssetUUID();
	}

	void ModelRenderer_AddMaterial(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint64_t materialID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Materials.push_back(UUID(materialID));
	}

	void ModelRenderer_SetMaterialAt(MonoEntityHandle* pEntityHandle, uint64_t componentID, size_t index, uint64_t materialID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		if (meshRenderer.m_Materials.size() <= index) return;
		meshRenderer.m_Materials[index] = UUID(materialID);
	}

	void ModelRenderer_ClearMaterials(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Materials.clear();
	}

	uint64_t ModelRenderer_GetModel(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		return meshRenderer.m_Model.AssetUUID();
	}

	void ModelRenderer_SetModel(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint64_t modelID)
	{
		ModelRenderer& meshRenderer = GetComponent<ModelRenderer>(pEntityHandle, componentID);
		meshRenderer.m_Model = UUID(modelID);
	}

#pragma endregion

#pragma region CameraComponent

	float CameraComponent_GetHalfFOV(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_HalfFOV;
	}

	void CameraComponent_SetHalfFOV(MonoEntityHandle* pEntityHandle, uint64_t componentID, float halfFov)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_HalfFOV = halfFov;
	}

	float CameraComponent_GetNear(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_Near;
	}

	void CameraComponent_SetNear(MonoEntityHandle* pEntityHandle, uint64_t componentID, float near)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_Near = near;
	}

	float CameraComponent_GetFar(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_Far;
	}

	void CameraComponent_SetFar(MonoEntityHandle* pEntityHandle, uint64_t componentID, float far)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_Far = far;
	}

	int CameraComponent_GetDisplayIndex(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_DisplayIndex;
	}

	void CameraComponent_SetDisplayIndex(MonoEntityHandle* pEntityHandle, uint64_t componentID, int displayIndex)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_DisplayIndex = displayIndex;
	}

	int CameraComponent_GetPriority(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_Priority;
	}

	void CameraComponent_SetPriority(MonoEntityHandle* pEntityHandle, uint64_t componentID, int priority)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_Priority = priority;
	}

	LayerMask CameraComponent_GetLayerMask(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_LayerMask;
	}

	void CameraComponent_SetLayerMask(MonoEntityHandle* pEntityHandle, uint64_t componentID, LayerMask* pLayerMask)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_LayerMask.m_Mask = pLayerMask->m_Mask;
	}

	glm::vec4 CameraComponent_GetClearColor(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_ClearColor;
	}

	void CameraComponent_SetClearColor(MonoEntityHandle* pEntityHandle, uint64_t componentID, glm::vec4* clearCol)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		cameraComp.m_ClearColor = *clearCol;
	}

	uint64_t CameraComponent_GetCameraID(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		return cameraComp.m_Camera.GetUUID();
	}
	
	void CameraComponent_PrepareNextPick(MonoEntityHandle* pEntityHandle, uint64_t componentID, glm::vec2* position)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
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
	
	PickResultWrapper CameraComponent_GetPickResult(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		RendererModule* pRenderer = Entity_EngineInstance->GetMainModule<RendererModule>();
		size_t resultIndex;
		if (!pRenderer->PickResultIndex(cameraComp.m_Camera.GetUUID(), resultIndex))
			return { 0, 0, Vec3Wrapper{{}}, Vec3Wrapper{{}} };
		const PickResult pickResult = pRenderer->GetPickResult(resultIndex);

		uint64_t objectID = 0;

		GScene* pScene = (GScene*)Entity_EngineInstance->GetSceneManager()->GetOpenScene(pickResult.m_Object.SceneUUID());
		if (pScene)
		{
			const Utils::ECS::EntityID entity = pScene->GetEntityByUUID(pickResult.m_Object.ObjectUUID()).GetEntityID();
			MonoEntityHandle handle{ entity, pickResult.m_Object.SceneUUID() };
			objectID = Entity_GetSceneObjectID(&handle);
		}
		return PickResultWrapper{ pickResult.m_CameraID, objectID,
			ToVec3Wrapper(pickResult.m_WorldPosition), ToVec3Wrapper(pickResult.m_Normal) };
	}
	
	Vec2Wrapper CameraComponent_GetResolution(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CameraComponent& cameraComp = GetComponent<CameraComponent>(pEntityHandle, componentID);
		const glm::uvec2& resolution = cameraComp.m_Camera.GetResolution();
		return ToVec2Wrapper(resolution);
	}

#pragma endregion

#pragma region Layer Component

	LayerWrapper LayerComponent_GetLayer(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		LayerComponent& layerComp = GetComponent<LayerComponent>(pEntityHandle, componentID);
		return LayerWrapper(layerComp.m_Layer.Layer(&Entity_EngineInstance->GetLayerManager()));
	}

	void LayerComponent_SetLayer(MonoEntityHandle* pEntityHandle, uint64_t componentID, LayerWrapper* layer)
	{
		LayerComponent& layerComp = GetComponent<LayerComponent>(pEntityHandle, componentID);
		const Layer* pLayer = Entity_EngineInstance->GetLayerManager().GetLayerByName(mono_string_to_utf8(layer->Name));
		layerComp.m_Layer = pLayer ? Entity_EngineInstance->GetLayerManager().GetLayerIndex(pLayer) + 1 : 0;
	}

#pragma endregion

#pragma region Light Component

	glm::vec4 LightComponent_GetColor(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		return lightComp.m_Color;
	}

	void LightComponent_SetColor(MonoEntityHandle* pEntityHandle, uint64_t componentID, glm::vec4* color)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		lightComp.m_Color = *color;
	}

	float LightComponent_GetIntensity(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		return lightComp.m_Intensity;
	}

	void LightComponent_SetIntensity(MonoEntityHandle* pEntityHandle, uint64_t componentID, float intensity)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		lightComp.m_Intensity = intensity;
	}

	float LightComponent_GetRange(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		return lightComp.m_Range;
	}

	void LightComponent_SetRange(MonoEntityHandle* pEntityHandle, uint64_t componentID, float range)
	{
		LightComponent& lightComp = GetComponent<LightComponent>(pEntityHandle, componentID);
		lightComp.m_Range = range;
	}

#pragma endregion

#pragma region MonoScriptComponent

	uint64_t MonoScriptComponent_GetScript(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		MonoScriptComponent& scriptComp = GetComponent<MonoScriptComponent>(pEntityHandle, componentID);
		return scriptComp.m_Script.AssetUUID();
	}

	void MonoScriptComponent_SetScript(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint64_t scriptID)
	{
		MonoScriptComponent& scriptComp = GetComponent<MonoScriptComponent>(pEntityHandle, componentID);
		if (scriptComp.m_Script.AssetUUID() != 0)
		{
			Entity_EngineInstance->GetDebug().LogError("You are trying to set the script on a MonoScriptComponent that already has a script, this is not allowed.");
			return;
		}
		scriptComp.m_Script = UUID(scriptID);
	}

	MonoObject* MonoScriptComponent_GetBehaviour(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		MonoScriptComponent& scriptComp = GetComponent<MonoScriptComponent>(pEntityHandle, componentID);
		MonoScriptObjectManager* pScriptObjectManager = MonoManager::Instance()->ActiveDomain()->ScriptObjectManager();
		/* TODO */
		return nullptr;
	}

#pragma endregion

#pragma region AudioSource

	uint64_t AudioSource_GetAudio(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		return source.m_Audio.AssetUUID();
	}

	void AudioSource_SetAudio(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint64_t audioID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		source.m_Audio = UUID(audioID);
	}

	bool AudioSource_GetAsMusic(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		return source.m_AsMusic;
	}

	void AudioSource_SetAsMusic(MonoEntityHandle* pEntityHandle, uint64_t componentID, bool asMusic)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		source.m_AsMusic = asMusic;
	}

	uint32_t AudioSource_GetLoops(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		return source.m_Loops;
	}

	void AudioSource_SetLoops(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint32_t loops)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		source.m_Loops = loops;
	}

	bool AudioSource_GetEnable3D(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		return source.m_Enable3D;
	}

	void AudioSource_SetEnable3D(MonoEntityHandle* pEntityHandle, uint64_t componentID, bool allow)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		source.m_Enable3D = allow;
	}

	bool AudioSource_GetAutoPlay(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		return source.m_AutoPlay;
	}

	void AudioSource_SetAutoPlay(MonoEntityHandle* pEntityHandle, uint64_t componentID, bool autoPlay)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		source.m_AutoPlay = autoPlay;
	}

	bool AudioSource_GetPlaying(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
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

	bool AudioSource_GetPaused(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
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

	void AudioSource_SetPaused(MonoEntityHandle* pEntityHandle, uint64_t componentID, bool pause)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		if (source.m_CurrentChannel == -1) return;
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_SetPaused > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(pEntityHandle);

		if (pause)
			AudioSourceSystem::Pause(&pScene->GetRegistry(), pEntityHandle->m_EntityID, source);
		else
			AudioSourceSystem::Resume(&pScene->GetRegistry(), pEntityHandle->m_EntityID, source);
	}

	float AudioSource_GetVolume(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		return source.m_Volume;
	}

	void AudioSource_SetVolume(MonoEntityHandle* pEntityHandle, uint64_t componentID, float volume)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		source.m_Volume = volume;
		if (source.m_CurrentChannel == -1) return;
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_SetVolume > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(pEntityHandle);
		AudioSourceSystem::UpdateVolume(&pScene->GetRegistry(), pEntityHandle->m_EntityID, source);
	}

	void AudioSource_Play(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_Play > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(pEntityHandle);
		AudioSourceSystem::Play(&pScene->GetRegistry(), pEntityHandle->m_EntityID, source);
	}

	void AudioSource_Stop(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_Stop > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(pEntityHandle);
		AudioSourceSystem::Stop(&pScene->GetRegistry(), pEntityHandle->m_EntityID, source);
	}

	void AudioSource_Pause(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_Pause > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(pEntityHandle);
		AudioSourceSystem::Stop(&pScene->GetRegistry(), pEntityHandle->m_EntityID, source);
	}

	void AudioSource_Resume(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		AudioModule* pAudioModule = Entity_EngineInstance->GetOptionalModule<AudioModule>();
		if (!pAudioModule)
		{
			Entity_EngineInstance->GetDebug().LogError("AudioSource_Resume > No audio module was loaded to play audio.");
			return;
		}
		GScene* pScene = GetEntityScene(pEntityHandle);
		AudioSourceSystem::Resume(&pScene->GetRegistry(), pEntityHandle->m_EntityID, source);
	}

	SpatializationSettings* AudioSource_GetSpatializationSettings(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		return &source.m_Spatialization;
	}

	AudioSourceSimulationSettings* AudioSource_GetSimulationSettings(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioSource& source = GetComponent<AudioSource>(pEntityHandle, componentID);
		return &source.m_Simulation;
	}

#pragma endregion

#pragma region Audio listener

	bool AudioListener_GetEnabled(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioListener& listener = GetComponent<AudioListener>(pEntityHandle, componentID);
		return listener.m_Enable;
	}

	void AudioListener_SetEnabled(MonoEntityHandle* pEntityHandle, uint64_t componentID, bool value)
	{
		AudioListener& listener = GetComponent<AudioListener>(pEntityHandle, componentID);
		listener.m_Enable = value;
	}

	AudioSimulationSettings* AudioListener_GetSimulationSettings(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		AudioListener& listener = GetComponent<AudioListener>(pEntityHandle, componentID);
		return &listener.m_Simulation;
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
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_GetEntityHandle", SceneObject_GetEntityHandle);
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

		/* MonoScriptComponent */
		BIND("GloryEngine.Entities.MonoScriptComponent::MonoScriptComponent_GetScript", MonoScriptComponent_GetScript);
		BIND("GloryEngine.Entities.MonoScriptComponent::MonoScriptComponent_SetScript", MonoScriptComponent_SetScript);
		BIND("GloryEngine.Entities.MonoScriptComponent::MonoScriptComponent_GetBehaviour", MonoScriptComponent_GetBehaviour);

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

		/* Entity Scene Object */
		BIND("GloryEngine.Entities.SceneObject::SceneObject_GetEntityHandle", SceneObject_GetEntityHandle);
	}

	void EntityCSAPI::SetEngine(Engine* pEngine)
	{
		Entity_EngineInstance = pEngine;
	}

	MonoEntityHandle::MonoEntityHandle() : m_EntityID(0), m_SceneID(0)
	{
	}

	MonoEntityHandle::MonoEntityHandle(uint64_t entityID, uint64_t sceneID) : m_EntityID(entityID), m_SceneID(sceneID)
	{
	}

#pragma endregion
}
