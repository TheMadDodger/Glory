#include "CoreCSAPI.h"
#include "GloryMonoScipting.h"
#include "MonoManager.h"
#include "MathCSAPI.h"

#include <Engine.h>
#include <SceneManager.h>
#include <MaterialManager.h>
#include <Layer.h>
#include <GameTime.h>
#include <LayerManager.h>
#include <EngineProfiler.h>
#include <AssetDatabase.h>
#include <AssetManager.h>
#include <ObjectManager.h>

#include <MaterialData.h>
#include <MaterialInstanceData.h>
#include <PrefabData.h>

namespace Glory
{
	Engine* Core_EngineInstance;

	LayerWrapper::LayerWrapper(const Layer* pLayer) : Mask(pLayer ? pLayer->m_Mask : 0),
		Name(mono_string_new(mono_domain_get(), pLayer ? pLayer->m_Name.c_str() : ""))
	{ }

#pragma region Debug

	void Log(MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp)
	{
		Core_EngineInstance->GetDebug().Log(mono_string_to_utf8(message), logLevel, bIncludeTimeStamp);
	}

	void LogInfo(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Core_EngineInstance->GetDebug().LogInfo(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogNotice(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Core_EngineInstance->GetDebug().LogNotice(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogWarning(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Core_EngineInstance->GetDebug().LogWarning(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogError(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Core_EngineInstance->GetDebug().LogError(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogFatalError(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Core_EngineInstance->GetDebug().LogFatalError(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogOnce(MonoString* key, MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp)
	{
		Core_EngineInstance->GetDebug().LogOnce(mono_string_to_utf8(key), mono_string_to_utf8(message), logLevel, bIncludeTimeStamp);
	}

	void DrawLine(Vec3Wrapper start, Vec3Wrapper end, Vec4Wrapper color, float time)
	{
		Core_EngineInstance->GetDebug().DrawLine(ToGLMVec3(start), ToGLMVec3(end), ToGLMVec4(color), time);
	}
	
	void DrawRay(Vec3Wrapper start, Vec3Wrapper dir, Vec4Wrapper color, float length, float time)
	{
		Core_EngineInstance->GetDebug().DrawRay(ToGLMVec3(start), ToGLMVec3(dir), ToGLMVec4(color), length, time);
	}

#pragma endregion

#pragma region Game Time

	float Time_GetDeltaTime()
	{
		return Core_EngineInstance->Time().GetDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetGameDeltaTime()
	{
		return Core_EngineInstance->Time().GetGameDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetGraphicsDeltaTime()
	{
		return Core_EngineInstance->Time().GetGraphicsDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetUnscaledDeltaTime()
	{
		return Core_EngineInstance->Time().GetUnscaledDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetUnscaledGameDeltaTime()
	{
		return Core_EngineInstance->Time().GetUnscaledGameDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetUnscaledGraphicsDeltaTime()
	{
		return Core_EngineInstance->Time().GetUnscaledGraphicsDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetCurrentTime()
	{
		return Core_EngineInstance->Time().GetTime();
	}

	float Time_GetUnscaledTime()
	{
		return Core_EngineInstance->Time().GetUnscaledTime();
	}

	float Time_GetFrameRate()
	{
		return Core_EngineInstance->Time().GetFrameRate();
	}

	int Time_GetTotalFrames()
	{
		return Core_EngineInstance->Time().GetTotalFrames();
	}

	int Time_GetTotalGameFrames()
	{
		return Core_EngineInstance->Time().GetTotalGameFrames();
	}

	float Time_GetTimeScale()
	{
		return Core_EngineInstance->Time().GetTimeScale();
	}

	void Time_SetTimeScale(float scale)
	{
		return Core_EngineInstance->Time().SetTimeScale(scale);
	}

#pragma endregion

#pragma region Layer Management

	void LayerManager_AddLayer(MonoString* name)
	{
		const std::string nameStr = mono_string_to_utf8(name);
		Core_EngineInstance->GetLayerManager().AddLayer(nameStr);
	}

	LayerWrapper LayerManager_GetLayerByName(MonoString* name)
	{
		const std::string nameStr = mono_string_to_utf8(name);
		const Layer* layer = Core_EngineInstance->GetLayerManager().GetLayerByName(nameStr);
		return { layer };
	}

	MonoString* LayerManager_LayerMaskToString(const LayerMask* layerMask)
	{
		const std::string str = Core_EngineInstance->GetLayerManager().LayerMaskToString(*layerMask);
		return mono_string_new(mono_domain_get(), str.c_str());
	}

	int LayerManager_GetLayerIndex(const Layer* pLayer)
	{
		return Core_EngineInstance->GetLayerManager().GetLayerIndex(pLayer);
	}

	LayerWrapper LayerManager_GetLayerAtIndex(int index)
	{
		const Layer* layer = Core_EngineInstance->GetLayerManager().GetLayerAtIndex(index);
		return { layer };
	}

#pragma endregion

#pragma region Profiler

	void Profiler_BeginSample(MonoString* name)
	{
		const std::string nameStr = mono_string_to_utf8(name);
		Core_EngineInstance->Profiler().BeginSample(nameStr);
	}

	void Profiler_EndSample()
	{
		Core_EngineInstance->Profiler().EndSample();
	}

#pragma endregion

#pragma region Objects API

	MonoString* Object_GetName(uint64_t uuid)
	{
		Object* pObject = Core_EngineInstance->GetObjectManager().Find(uuid);
		if (!pObject) return nullptr;
		return mono_string_new(mono_domain_get(), pObject->Name().c_str());
	}

	void Object_SetName(uint64_t uuid, MonoString* name)
	{
		Object* pObject = Core_EngineInstance->GetObjectManager().Find(uuid);
		if (!pObject) return;
		const std::string nameStr = mono_string_to_utf8(name);
		pObject->SetName(nameStr);
	}

	MonoString* Resource_GetName(uint64_t uuid)
	{
		Resource* pResource = Core_EngineInstance->GetAssetManager().GetAssetImmediate(uuid);
		if (!pResource) return nullptr;
		return mono_string_new(mono_domain_get(), pResource->Name().c_str());
	}

	void Resource_SetName(uint64_t uuid, MonoString* name)
	{
		Resource* pResource = Core_EngineInstance->GetAssetManager().GetAssetImmediate(uuid);
		if (!pResource) return;
		const std::string nameStr = mono_string_to_utf8(name);
		pResource->SetName(nameStr);
	}

#pragma endregion

#pragma region Materials

	template<typename T>
	void Material_Set(uint64_t matID, MonoString* propName, T value)
	{
		MaterialData* pMaterial = Core_EngineInstance->GetAssetManager().GetAssetImmediate<MaterialData>(matID);
		if (!pMaterial)
		{
			Core_EngineInstance->GetDebug().LogError("Material does not exist!");
			return;
		}
		const std::string propNameStr = mono_string_to_utf8(propName);
		pMaterial->Set<T>(Core_EngineInstance->GetMaterialManager(), propNameStr, value);
	}

	template<typename T>
	bool Material_Get(uint64_t matID, MonoString* propName, T value)
	{
		MaterialData* pMaterial = Core_EngineInstance->GetAssetManager().GetAssetImmediate<MaterialData>(matID);
		if (!pMaterial)
		{
			Core_EngineInstance->GetDebug().LogError("Material does not exist!");
			return false;
		}
		const std::string propNameStr = mono_string_to_utf8(propName);
		return pMaterial->Get<T>(Core_EngineInstance->GetMaterialManager(), propNameStr, value);
	}

	void Material_SetTexture(uint64_t matID, MonoString* propName, uint64_t value)
	{
		const auto pMaterial = Core_EngineInstance->GetAssetManager().GetAssetImmediate<MaterialData>(matID);
		if (!pMaterial)
		{
			Core_EngineInstance->GetDebug().LogError("Material does not exist!");
			return;
		}
		const std::string propNameStr = mono_string_to_utf8(propName);
		TextureData* pImage = value ? Core_EngineInstance->GetAssetManager().GetAssetImmediate<TextureData>(value) : nullptr;
		pMaterial->SetTexture(Core_EngineInstance->GetMaterialManager(), propNameStr, pImage);
	}

	bool Material_GetTexture(uint64_t matID, MonoString* propName, uint64_t& value)
	{
		AssetManager& pManager = Core_EngineInstance->GetAssetManager();
		const auto pMaterial = pManager.GetAssetImmediate<MaterialData>(matID);
		if (!pMaterial)
		{
			Core_EngineInstance->GetDebug().LogError("Material does not exist!");
			return false;
		}
		const std::string propNameStr = mono_string_to_utf8(propName);
		TextureData* pImage = nullptr;
		if (!pMaterial->GetTexture(Core_EngineInstance->GetMaterialManager(), propNameStr, &pImage, &pManager)) return false;
		value = pImage ? pImage->GetUUID() : 0;
		return true;
	}
	
	uint64_t Material_CreateInstance(uint64_t matID)
	{
		AssetManager& pManager = Core_EngineInstance->GetAssetManager();
		MaterialManager& materials = Core_EngineInstance->GetMaterialManager();
		auto pMaterial = pManager.GetAssetImmediate<MaterialData>(matID);
		if (!pMaterial)
		{
			Core_EngineInstance->GetDebug().LogError("Material does not exist!");
			return 0;
		}
		if (pMaterial->IsInstance())
		{
			pMaterial = static_cast<MaterialInstanceData*>(pMaterial)->GetBaseMaterial(materials);
		}
		if (!pMaterial)
		{
			Core_EngineInstance->GetDebug().LogError("Material does not exist!");
			return 0;
		}
		MaterialInstanceData* pInstance = materials.CreateRuntimeMaterialInstance(pMaterial->GetUUID());
		return pInstance->GetUUID();
	}

#pragma endregion

#pragma region Scenes

	uint64_t Scene_NewEmptyObject(uint64_t sceneID)
	{
		GScene* pScene = Core_EngineInstance->GetSceneManager()->GetOpenScene(UUID(sceneID));
		if(!pScene) return 0;
		const Entity entity = pScene->CreateEmptyObject();
		if (!entity.IsValid()) return 0;
		return pScene->GetEntityUUID(entity.GetEntityID());
	}

	uint64_t Scene_NewEmptyObjectWithName(uint64_t sceneID, MonoString* name)
	{
		GScene* pScene = Core_EngineInstance->GetSceneManager()->GetOpenScene(UUID(sceneID));
		if (!pScene) return 0;
		const std::string nameStr = mono_string_to_utf8(name);
		const Entity entity = pScene->CreateEmptyObject(nameStr, UUID());
		if (!entity.IsValid()) return 0;
		return pScene->GetEntityUUID(entity.GetEntityID());
	}

	size_t Scene_ObjectsCount(uint64_t sceneID)
	{
		GScene* pScene = Core_EngineInstance->GetSceneManager()->GetOpenScene(UUID(sceneID));
		if (!pScene) return 0;
		return pScene->SceneObjectsCount();
	}

	void Scene_Destroy(uint64_t sceneID, uint64_t objectID)
	{
		GScene* pScene = Core_EngineInstance->GetSceneManager()->GetOpenScene(UUID(sceneID));
		if (!pScene) return;
		const Entity entity = pScene->GetEntityByUUID(UUID(objectID));
		if (!entity.IsValid()) return;
		pScene->DestroyEntity(entity.GetEntityID());
	}

	uint64_t Scene_InstantiatePrefab(uint64_t sceneID, uint64_t prefabID, Vec3Wrapper position, QuatWrapper rotation, Vec3Wrapper scale, uint64_t parentID)
	{
		PrefabData* pPrefab = Core_EngineInstance->GetAssetManager().GetAssetImmediate<PrefabData>(prefabID);
		if (!pPrefab) return 0;
		GScene* pScene = Core_EngineInstance->GetSceneManager()->GetOpenScene(UUID(sceneID));
		if (!pScene) return 0;
		const Entity parentEntity = pScene->GetEntityByUUID(UUID(parentID));
		const Entity entity = pScene->InstantiatePrefab(parentEntity.IsValid() ? pScene->GetEntityUUID(parentEntity.GetEntityID()) : 0, pPrefab, ToGLMVec3(position), ToGLMQuat(rotation), ToGLMVec3(scale));
		if (!entity.IsValid()) return 0;
		return pScene->GetEntityUUID(entity.GetEntityID());
	}

#pragma endregion

#pragma region Scene Management

	uint64_t SceneManager_CreateEmptyScene(MonoString* name)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->NewScene(mono_string_to_utf8(name));
		return pScene->GetUUID();
	}

	size_t SceneManager_OpenScenesCount()
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return 0;
		return pScenes->OpenScenesCount();
	}

	uint64_t SceneManager_GetOpenSceneAt(size_t index)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(index);
		return pScene->GetUUID();
	}

	uint64_t SceneManager_GetOpenScene(uint64_t sceneID)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		return pScene->GetUUID();
	}

	uint64_t SceneManager_GetActiveScene()
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetActiveScene();
		return pScene->GetUUID();
	}

	void SceneManager_SetActiveScene(uint64_t sceneID)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return;
		pScenes->SetActiveScene(pScene);
	}

	void SceneManager_CloseAllScenes()
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return;
		pScenes->CloseAllScenes();
	}

	void SceneManager_OpenScene(uint64_t id, bool additive)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return;
		pScenes->OpenScene(id, additive);
	}

	void SceneManager_OpenSceneByName(MonoString* name, bool additive)
	{
		const UUID sceneID = Core_EngineInstance->GetAssetDatabase().FindSceneID(mono_string_to_utf8(name));
		if (!sceneID) return;
		SceneManager_OpenScene(sceneID, additive);
	}

	void SceneManager_CloseScene(uint64_t sceneID)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return;
		pScene->MarkForDestruction();
	}

#pragma endregion

#pragma region Scene Objects

	MonoString* SceneObject_GetName(uint64_t sceneID, uint64_t objectID)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return nullptr;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return nullptr;
		const Entity entity = pScene->GetEntityByUUID(UUID(objectID));
		return entity.IsValid() ? mono_string_new(mono_domain_get(), pScene->EntityName(entity.GetEntityID()).data()) : nullptr;
	}

	void SceneObject_SetName(uint64_t sceneID, uint64_t objectID, MonoString* name)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return;
		const Entity entity = pScene->GetEntityByUUID(UUID(objectID));
		if (!entity.IsValid()) return;
		pScene->SetEntityName(entity.GetEntityID(), std::string{mono_string_to_utf8(name)});
	}

	size_t SceneObject_GetSiblingIndex(uint64_t sceneID, uint64_t objectID)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return 0;
		const Entity entity = pScene->GetEntityByUUID(UUID(objectID));
		if (!entity.IsValid()) return 0;
		return pScene->SiblingIndex(entity.GetEntityID());
	}

	void SceneObject_SetSiblingIndex(uint64_t sceneID, uint64_t objectID, size_t index)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return;
		const Entity entity = pScene->GetEntityByUUID(UUID(objectID));
		if (!entity.IsValid()) return;
		pScene->SetSiblingIndex(entity.GetEntityID(), index);
	}

	size_t SceneObject_GetChildCount(uint64_t sceneID, uint64_t objectID)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return 0;
		const Entity entity = pScene->GetEntityByUUID(UUID(objectID));
		if (!entity.IsValid()) return 0;
		return pScene->ChildCount(entity.GetEntityID());
	}

	uint64_t SceneObject_GetChild(uint64_t sceneID, uint64_t objectID, size_t index)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return 0;
		const Entity entity = pScene->GetEntityByUUID(UUID(objectID));
		if (!entity.IsValid()) return 0;
		if (index >= entity.ChildCount()) return 0;
		const Entity childEntity = { pScene->Child(entity.GetEntityID(), index), pScene};
		return childEntity.IsValid() ? pScene->GetEntityUUID(childEntity.GetEntityID()) : 0;
	}

	uint64_t SceneObject_GetParent(uint64_t sceneID, uint64_t objectID)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return 0;
		const Entity entity = pScene->GetEntityByUUID(UUID(objectID));
		if (!entity.IsValid()) return 0;
		Utils::ECS::EntityID parent = pScene->Parent(entity.GetEntityID());
		if (!parent) return 0;
		Entity parentEntity = pScene->GetEntityByEntityID(parent);
		return parentEntity.IsValid() ? pScene->GetEntityUUID(parent) : 0;
	}

	void SceneObject_SetParent(uint64_t sceneID, uint64_t objectID, uint64_t parentID)
	{
		SceneManager* pScenes = Core_EngineInstance->GetSceneManager();
		if (!pScenes) return;
		GScene* pScene = pScenes->GetOpenScene(UUID(sceneID));
		if (!pScene) return;
		const Entity entity = pScene->GetEntityByUUID(UUID(objectID));
		const Entity parent = pScene->GetEntityByUUID(UUID(parentID));
		if (!entity.IsValid() || !parent.IsValid()) return;
		pScene->SetParent(entity.GetEntityID(), parent.GetEntityID());
	}

#pragma endregion

#pragma region Binding

	void CoreCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		// Debug
		BIND("GloryEngine.Debug::Log(string,GloryEngine.LogLevel,bool)", Log);
		BIND("GloryEngine.Debug::LogInfo(string,bool)", LogInfo);
		BIND("GloryEngine.Debug::LogNotice(string,bool)", LogNotice);
		BIND("GloryEngine.Debug::LogWarning(string,bool)", LogWarning);
		BIND("GloryEngine.Debug::LogError(string,bool)", LogError);
		BIND("GloryEngine.Debug::LogFatalError(string,bool)", LogFatalError);
		BIND("GloryEngine.Debug::LogOnce(string,string,GloryEngine.LogLevel,bool)", LogOnce);
		BIND("GloryEngine.Debug::DrawLine", DrawLine);
		BIND("GloryEngine.Debug::DrawRay", DrawRay);

		// Time
		BIND("GloryEngine.Time::Time_GetDeltaTime", Time_GetDeltaTime);
		BIND("GloryEngine.Time::Time_GetGameDeltaTime", Time_GetGameDeltaTime);
		BIND("GloryEngine.Time::Time_GetGraphicsDeltaTime", Time_GetGraphicsDeltaTime);
		BIND("GloryEngine.Time::Time_GetUnscaledDeltaTime", Time_GetUnscaledDeltaTime);
		BIND("GloryEngine.Time::Time_GetUnscaledGameDeltaTime", Time_GetUnscaledGameDeltaTime);
		BIND("GloryEngine.Time::Time_GetUnscaledGraphicsDeltaTime", Time_GetUnscaledGraphicsDeltaTime);
		BIND("GloryEngine.Time::Time_GetCurrentTime", Time_GetCurrentTime);
		BIND("GloryEngine.Time::Time_GetUnscaledTime", Time_GetUnscaledTime);
		BIND("GloryEngine.Time::Time_GetFrameRate", Time_GetFrameRate);
		BIND("GloryEngine.Time::Time_GetTotalFrames", Time_GetTotalFrames);
		BIND("GloryEngine.Time::Time_GetTotalGameFrames", Time_GetTotalGameFrames);
		BIND("GloryEngine.Time::Time_SetTimeScale", Time_SetTimeScale);
		BIND("GloryEngine.Time::Time_GetTimeScale", Time_GetTimeScale);

		// Layers
		BIND("GloryEngine.LayerManager::AddLayer", LayerManager_AddLayer);
		BIND("GloryEngine.LayerManager::GetLayerByName", LayerManager_GetLayerByName);
		BIND("GloryEngine.LayerManager::LayerMaskToString", LayerManager_LayerMaskToString);
		BIND("GloryEngine.LayerManager::GetLayerIndex", LayerManager_GetLayerIndex);
		BIND("GloryEngine.LayerManager::GetLayerAtIndex", LayerManager_GetLayerAtIndex);

		// Profiler
		BIND("GloryEngine.Profiler::BeginSample", Profiler_BeginSample);
		BIND("GloryEngine.Profiler::EndSample", Profiler_EndSample);

		// Objects
		BIND("GloryEngine.Object::Object_SetName", Object_SetName);
		BIND("GloryEngine.Object::Object_GetName", Object_GetName);

		// Resources
		BIND("GloryEngine.Resource::Resource_SetName", Resource_SetName);
		BIND("GloryEngine.Resource::Resource_GetName", Resource_GetName);

		// Materials
		BIND("GloryEngine.Material::Material_SetFloat", Material_Set<float>);
		BIND("GloryEngine.Material::Material_GetFloat", Material_Get<float>);
		BIND("GloryEngine.Material::Material_SetDouble", Material_Set<double>);
		BIND("GloryEngine.Material::Material_GetDouble", Material_Get<double>);
		BIND("GloryEngine.Material::Material_SetInt", Material_Set<int>);
		BIND("GloryEngine.Material::Material_GetInt", Material_Get<int>);
		BIND("GloryEngine.Material::Material_SetUInt", Material_Set<unsigned int>);
		BIND("GloryEngine.Material::Material_GetUInt", Material_Get<unsigned int>);
		BIND("GloryEngine.Material::Material_SetBool", Material_Set<bool>);
		BIND("GloryEngine.Material::Material_GetBool", Material_Get<bool>);

		BIND("GloryEngine.Material::Material_SetVec2", Material_Set<glm::vec2>);
		BIND("GloryEngine.Material::Material_GetVec2", Material_Get<glm::vec2>);
		BIND("GloryEngine.Material::Material_SetVec3", Material_Set<glm::vec3>);
		BIND("GloryEngine.Material::Material_GetVec3", Material_Get<glm::vec3>);
		BIND("GloryEngine.Material::Material_SetVec4", Material_Set<glm::vec4>);
		BIND("GloryEngine.Material::Material_GetVec4", Material_Get<glm::vec4>);
		BIND("GloryEngine.Material::Material_SetIVec2", Material_Set<glm::ivec2>);
		BIND("GloryEngine.Material::Material_GetIVec2", Material_Get<glm::ivec2>);
		BIND("GloryEngine.Material::Material_SetIVec3", Material_Set<glm::ivec3>);
		BIND("GloryEngine.Material::Material_GetIVec3", Material_Get<glm::ivec3>);
		BIND("GloryEngine.Material::Material_SetIVec4", Material_Set<glm::ivec4>);
		BIND("GloryEngine.Material::Material_GetIVec4", Material_Get<glm::ivec4>);
		BIND("GloryEngine.Material::Material_SetUVec2", Material_Set<glm::uvec2>);
		BIND("GloryEngine.Material::Material_GetUVec2", Material_Get<glm::uvec2>);
		BIND("GloryEngine.Material::Material_SetUVec3", Material_Set<glm::uvec3>);
		BIND("GloryEngine.Material::Material_GetUVec3", Material_Get<glm::uvec3>);
		BIND("GloryEngine.Material::Material_SetUVec4", Material_Set<glm::uvec4>);
		BIND("GloryEngine.Material::Material_GetUVec4", Material_Get<glm::uvec4>);

		BIND("GloryEngine.Material::Material_SetTexture", Material_SetTexture);
		BIND("GloryEngine.Material::Material_GetTexture", Material_GetTexture);

		BIND("GloryEngine.Material::Material_CreateInstance", Material_CreateInstance);

		// Scenes
		BIND("GloryEngine.SceneManagement.Scene::Scene_NewEmptyObject", Scene_NewEmptyObject);
		BIND("GloryEngine.SceneManagement.Scene::Scene_NewEmptyObjectWithName", Scene_NewEmptyObjectWithName);
		BIND("GloryEngine.SceneManagement.Scene::Scene_ObjectsCount", Scene_ObjectsCount);
		BIND("GloryEngine.SceneManagement.Scene::Scene_Destroy", Scene_Destroy);
		BIND("GloryEngine.SceneManagement.Scene::Scene_InstantiatePrefab", Scene_InstantiatePrefab);

		// Scene Manager
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_CreateEmptyScene", SceneManager_CreateEmptyScene);
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_OpenScenesCount", SceneManager_OpenScenesCount);
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_GetOpenSceneAt", SceneManager_GetOpenSceneAt);
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_GetOpenScene", SceneManager_GetOpenScene);
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_GetActiveScene", SceneManager_GetActiveScene);
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_SetActiveScene", SceneManager_SetActiveScene);
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_CloseAllScenes", SceneManager_CloseAllScenes);
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_OpenScene", SceneManager_OpenScene);
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_OpenSceneByName", SceneManager_OpenSceneByName);
		BIND("GloryEngine.SceneManagement.SceneManager::SceneManager_CloseScene", SceneManager_CloseScene);

		// Scene Objects
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_GetName", SceneObject_GetName);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_SetName", SceneObject_SetName);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_GetSiblingIndex", SceneObject_GetSiblingIndex);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_SetSiblingIndex", SceneObject_SetSiblingIndex);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_GetChildCount", SceneObject_GetChildCount);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_GetChild", SceneObject_GetChild);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_GetParent", SceneObject_GetParent);
		BIND("GloryEngine.SceneManagement.SceneObject::SceneObject_SetParent", SceneObject_SetParent);
	}

	void CoreCSAPI::SetEngine(Engine* pEngine)
	{
		Core_EngineInstance = pEngine;
	}

	CoreCSAPI::CoreCSAPI() {}
	CoreCSAPI::~CoreCSAPI() {}

#pragma endregion
}