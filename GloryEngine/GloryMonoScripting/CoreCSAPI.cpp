#include "CoreCSAPI.h"
#include "GloryMonoScipting.h"
#include "MonoManager.h"
#include <GameTime.h>
#include <LayerManager.h>
#include <EngineProfiler.h>
#include <AssetManager.h>

namespace Glory
{
#pragma region Debug

	void Log(MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp)
	{
		Debug::Log(mono_string_to_utf8(message), logLevel, bIncludeTimeStamp);
	}

	void LogInfo(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogInfo(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogNotice(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogNotice(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogWarning(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogWarning(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogError(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogError(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogFatalError(MonoString* message, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogFatalError(mono_string_to_utf8(message), bIncludeTimeStamp);
	}

	void LogOnce(MonoString* key, MonoString* message, Debug::LogLevel logLevel, MonoBoolean bIncludeTimeStamp)
	{
		Debug::LogOnce(mono_string_to_utf8(key), mono_string_to_utf8(message), logLevel, bIncludeTimeStamp);
	}

#pragma endregion

#pragma region Game Time

	float Time_GetDeltaTime()
	{
		return Time::GetDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetGameDeltaTime()
	{
		return Time::GetGameDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetGraphicsDeltaTime()
	{
		return Time::GetGraphicsDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetUnscaledDeltaTime()
	{
		return Time::GetUnscaledDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetUnscaledGameDeltaTime()
	{
		return Time::GetUnscaledGameDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetUnscaledGraphicsDeltaTime()
	{
		return Time::GetUnscaledGraphicsDeltaTime<float, std::ratio<1, 1>>();
	}

	float Time_GetCurrentTime()
	{
		return Time::GetTime();
	}

	float Time_GetUnscaledTime()
	{
		return Time::GetUnscaledTime();
	}

	float Time_GetFrameRate()
	{
		return Time::GetFrameRate();
	}

	int Time_GetTotalFrames()
	{
		return Time::GetTotalFrames();
	}

	int Time_GetTotalGameFrames()
	{
		return Time::GetTotalGameFrames();
	}

	float Time_GetTimeScale()
	{
		return Time::GetTimeScale();
	}

	void Time_SetTimeScale(float scale)
	{
		return Time::SetTimeScale(scale);
	}

#pragma endregion

#pragma region Layer Management
	void LayerManager_AddLayer(MonoString* name)
	{
		const std::string nameStr = mono_string_to_utf8(name);
		LayerManager::AddLayer(nameStr);
	}

	void LayerManager_Load()
	{
		LayerManager::Load();
	}

	void LayerManager_Save()
	{
		LayerManager::Save();
	}

	LayerWrapper LayerManager_GetLayerByName(MonoString* name)
	{
		const std::string nameStr = mono_string_to_utf8(name);
		const Layer* layer = LayerManager::GetLayerByName(nameStr);
		return { layer };
	}

	MonoString* LayerManager_LayerMaskToString(const LayerMask* layerMask)
	{
		const std::string str = LayerManager::LayerMaskToString(*layerMask);
		return mono_string_new(MonoManager::GetDomain(), str.c_str());
	}

	int LayerManager_GetLayerIndex(const Layer* pLayer)
	{
		return LayerManager::GetLayerIndex(pLayer);
	}

	LayerWrapper LayerManager_GetLayerAtIndex(int index)
	{
		const Layer* layer = LayerManager::GetLayerAtIndex(index);
		return { layer };
	}

#pragma endregion

#pragma region Profiler

	void Profiler_BeginSample(MonoString* name)
	{
		const std::string nameStr = mono_string_to_utf8(name);
		Profiler::BeginSample(nameStr);
	}

	void Profiler_EndSample()
	{
		Profiler::EndSample();
	}

#pragma endregion

#pragma region Objects API

	MonoString* Object_GetName(UUID uuid)
	{
		Object* pObject = Object::FindObject(uuid);
		if (!pObject) return nullptr;
		return mono_string_new(MonoManager::GetDomain(), pObject->Name().c_str());
	}

	void Object_SetName(UUID uuid, MonoString* name)
	{
		Object* pObject = Object::FindObject(uuid);
		if (!pObject) return;
		const std::string nameStr = mono_string_to_utf8(name);
		pObject->SetName(nameStr);
	}

	MonoString* Resource_GetName(UUID uuid)
	{
		Resource* pResource = AssetManager::GetAssetImmediate(uuid);
		if (!pResource) return nullptr;
		return mono_string_new(MonoManager::GetDomain(), pResource->Name().c_str());
	}

	void Resource_SetName(UUID uuid, MonoString* name)
	{
		Resource* pResource = AssetManager::GetAssetImmediate(uuid);
		if (!pResource) return;
		const std::string nameStr = mono_string_to_utf8(name);
		pResource->SetName(nameStr);
	}

#pragma endregion

#pragma region Materials

	template<typename T>
	void Material_Set(UUID matID, MonoString* propName, T value)
	{
		MaterialData* pMaterial = AssetManager::GetAssetImmediate<MaterialData>(matID);
		if (!pMaterial)
		{
			Debug::LogError("Material does not exist!");
			return;
		}
		const std::string propNameStr = mono_string_to_utf8(propName);
		pMaterial->Set<T>(propNameStr, value);
	}

	template<typename T>
	bool Material_Get(UUID matID, MonoString* propName, T value)
	{
		MaterialData* pMaterial = AssetManager::GetAssetImmediate<MaterialData>(matID);
		if (!pMaterial)
		{
			Debug::LogError("Material does not exist!");
			return false;
		}
		const std::string propNameStr = mono_string_to_utf8(propName);
		return pMaterial->Get<T>(propNameStr, value);
	}

	void Material_SetTexture(UUID matID, MonoString* propName, UUID value)
	{
		const auto pMaterial = AssetManager::GetAssetImmediate<MaterialData>(matID);
		if (!pMaterial)
		{
			Debug::LogError("Material does not exist!");
			return;
		}
		const std::string propNameStr = mono_string_to_utf8(propName);
		ImageData* pImage = value ? AssetManager::GetAssetImmediate<ImageData>(value) : nullptr;
		pMaterial->SetTexture(propNameStr, pImage);
	}

	bool Material_GetTexture(UUID matID, MonoString* propName, UUID& value)
	{
		const auto pMaterial = AssetManager::GetAssetImmediate<MaterialData>(matID);
		if (!pMaterial)
		{
			Debug::LogError("Material does not exist!");
			return false;
		}
		const std::string propNameStr = mono_string_to_utf8(propName);
		ImageData* pImage = nullptr;
		if (!pMaterial->GetTexture(propNameStr, &pImage)) return false;
		value = pImage ? pImage->GetUUID() : 0;
		return true;
	}

#pragma endregion

#pragma region Scenes

	UUID Scene_NewEmptyObject(UUID sceneID)
	{
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(sceneID);
		if(!pScene) return 0;
		SceneObject* pNewObject = pScene->CreateEmptyObject();
		return pNewObject->GetUUID();
	}

	UUID Scene_NewEmptyObjectWithName(UUID sceneID, MonoString* name)
	{
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(sceneID);
		if (!pScene) return 0;
		const std::string nameStr = mono_string_to_utf8(name);
		SceneObject* pNewObject = pScene->CreateEmptyObject(nameStr, UUID());
		return pNewObject->GetUUID();
	}

	size_t Scene_ObjectsCount(UUID sceneID)
	{
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(sceneID);
		if (!pScene) return 0;
		return pScene->SceneObjectsCount();
	}

	UUID Scene_GetSceneObject(UUID sceneID, size_t index)
	{
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(sceneID);
		if (!pScene) return 0;
		SceneObject* pNewObject = pScene->GetSceneObject(index);
		return pNewObject ? pNewObject->GetUUID() : 0;
	}

	void Scene_Destroy(UUID sceneID, UUID objectID)
	{
		GScene* pScene = Game::GetGame().GetEngine()->GetScenesModule()->GetOpenScene(sceneID);
		if (!pScene) return;
		SceneObject* pObject = pScene->GetSceneObject(objectID);
		if (!pObject) return;
		pScene->DeleteObject(pObject);
	}

#pragma endregion

#pragma region Scene Management

	UUID SceneManager_CreateEmptyScene(MonoString* name)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->CreateEmptyScene(mono_string_to_utf8(name));
		return pScene ? pScene->GetUUID() : 0;
	}

	size_t SceneManager_OpenScenesCount()
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return 0;
		return pScenes->OpenScenesCount();
	}

	UUID SceneManager_GetOpenScene(size_t index)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(index);
		return pScene ? pScene->GetUUID() : 0;
	}

	UUID SceneManager_GetOpenSceneByUUID(UUID sceneID)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		return pScene ? pScene->GetUUID() : 0;
	}

	UUID SceneManager_GetActiveScene()
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetActiveScene();
		return pScene ? pScene->GetUUID() : 0;
	}

	void SceneManager_SetActiveScene(UUID sceneID)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		if (!pScene) return;
		pScenes->SetActiveScene(pScene);
	}

	void SceneManager_CloseAllScenes()
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return;
		pScenes->CloseAllScenes();
	}

	void SceneManager_OpenScene(MonoString* path)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return;
		pScenes->OpenScene(mono_string_to_utf8(path));
	}

	void SceneManager_CloseScene(UUID sceneID)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return;
		pScenes->CloseScene(sceneID);
	}

#pragma endregion

#pragma region Scene Objects

	MonoString* SceneObject_GetName(UUID objectID, UUID sceneID)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return nullptr;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		if (!pScene) return nullptr;
		SceneObject* pSceneObject = pScene->FindSceneObject(objectID);
		return pSceneObject ? mono_string_new(MonoManager::GetDomain(), pSceneObject->Name().c_str()) : nullptr;
	}

	void SceneObject_SetName(UUID objectID, UUID sceneID, MonoString* name)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		if (!pScene) return;
		SceneObject* pSceneObject = pScene->FindSceneObject(objectID);
		if (!pSceneObject) return;
		pSceneObject->SetName(mono_string_to_utf8(name));
	}

	size_t SceneObject_GetSiblingIndex(UUID objectID, UUID sceneID)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		if (!pScene) return 0;
		SceneObject* pSceneObject = pScene->FindSceneObject(objectID);
		if (!pSceneObject) return 0;
		return pSceneObject->GetSiblingIndex();
	}

	void SceneObject_SetSiblingIndex(UUID objectID, UUID sceneID, size_t index)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		if (!pScene) return;
		SceneObject* pSceneObject = pScene->FindSceneObject(objectID);
		if (!pSceneObject) return;
		pSceneObject->SetSiblingIndex(index);
	}

	size_t SceneObject_GetChildCount(UUID objectID, UUID sceneID)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		if (!pScene) return 0;
		SceneObject* pSceneObject = pScene->FindSceneObject(objectID);
		if (!pSceneObject) return 0;
		return pSceneObject->ChildCount();
	}

	UUID SceneObject_GetChild(UUID objectID, UUID sceneID, size_t index)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		if (!pScene) return 0;
		SceneObject* pSceneObject = pScene->FindSceneObject(objectID);
		if (!pSceneObject) return 0;
		SceneObject* pChild = pSceneObject->GetChild(index);
		return pChild ? pChild->GetUUID() : 0;
	}

	UUID SceneObject_GetParent(UUID objectID, UUID sceneID)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return 0;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		if (!pScene) return 0;
		SceneObject* pSceneObject = pScene->FindSceneObject(objectID);
		if (!pSceneObject) return 0;
		SceneObject* pParent = pSceneObject->GetParent();
		return pParent ? pParent->GetUUID() : 0;
	}

	void SceneObject_SetParent(UUID objectID, UUID sceneID, UUID parentID)
	{
		ScenesModule* pScenes = Game::GetGame().GetEngine()->GetScenesModule();
		if (!pScenes) return;
		GScene* pScene = pScenes->GetOpenScene(sceneID);
		if (!pScene) return;
		SceneObject* pSceneObject = pScene->FindSceneObject(objectID);
		if (!pSceneObject) return;
		SceneObject* pParent = pScene->FindSceneObject(parentID);
		if (!pParent) return;
		pSceneObject->SetParent(pParent);
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
		BIND("GloryEngine.LayerManager::Load", LayerManager_Load);
		BIND("GloryEngine.LayerManager::Save", LayerManager_Save);
		BIND("GloryEngine.LayerManager::GetLayerByName", LayerManager_GetLayerByName);
		BIND("GloryEngine.LayerManager::LayerMaskToString", LayerManager_LayerMaskToString);
		BIND("GloryEngine.LayerManager::GetLayerIndex", LayerManager_GetLayerIndex);
		BIND("GloryEngine.LayerManager::GetLayerAtIndex", LayerManager_GetLayerAtIndex);

		// Profiler
		BIND("GloryEngine.Profiler::BeginSample", Profiler_BeginSample);
		BIND("GloryEngine.Profiler::EndSample", Profiler_EndSample);

		// Objects
		BIND("GloryEngine.Resource::Object_SetName", Object_SetName);
		BIND("GloryEngine.Resource::Object_GetName", Object_GetName);

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

		// Scenes
		BIND("GloryEngine.Scene::Scene_NewEmptyObject", Scene_NewEmptyObject);
		BIND("GloryEngine.Scene::Scene_NewEmptyObjectWithName", Scene_NewEmptyObjectWithName);
		BIND("GloryEngine.Scene::Scene_ObjectsCount", Scene_ObjectsCount);
		BIND("GloryEngine.Scene::Scene_GetSceneObject", Scene_GetSceneObject);
		BIND("GloryEngine.Scene::Scene_Destroy", Scene_Destroy);

		// Scene Manager
		BIND("GloryEngine.SceneManager::SceneManager_CreateEmptyScene", SceneManager_CreateEmptyScene);
		BIND("GloryEngine.SceneManager::SceneManager_OpenScenesCount", SceneManager_OpenScenesCount);
		BIND("GloryEngine.SceneManager::SceneManager_GetOpenScene", SceneManager_GetOpenScene);
		BIND("GloryEngine.SceneManager::SceneManager_GetOpenSceneByUUID", SceneManager_GetOpenScene);
		BIND("GloryEngine.SceneManager::SceneManager_GetActiveScene", SceneManager_GetActiveScene);
		BIND("GloryEngine.SceneManager::SceneManager_SetActiveScene", SceneManager_SetActiveScene);
		BIND("GloryEngine.SceneManager::SceneManager_CloseAllScenes", SceneManager_CloseAllScenes);
		BIND("GloryEngine.SceneManager::SceneManager_OpenScene", SceneManager_OpenScene);
		BIND("GloryEngine.SceneManager::SceneManager_CloseScene", SceneManager_CloseScene);

		// Scene Objects
		BIND("GloryEngine.SceneObject::SceneObject_GetName", SceneObject_GetName);
		BIND("GloryEngine.SceneObject::SceneObject_SetName", SceneObject_SetName);
		BIND("GloryEngine.SceneObject::SceneObject_GetSiblingIndex", SceneObject_GetSiblingIndex);
		BIND("GloryEngine.SceneObject::SceneObject_SetSiblingIndex", SceneObject_SetSiblingIndex);
		BIND("GloryEngine.SceneObject::SceneObject_GetChildCount", SceneObject_GetChildCount);
		BIND("GloryEngine.SceneObject::SceneObject_GetChild", SceneObject_GetChild);
		BIND("GloryEngine.SceneObject::SceneObject_GetParent", SceneObject_GetParent);
		BIND("GloryEngine.SceneObject::SceneObject_SetParent", SceneObject_SetParent);
	}

	CoreCSAPI::CoreCSAPI() {}
	CoreCSAPI::~CoreCSAPI() {}

#pragma endregion
}