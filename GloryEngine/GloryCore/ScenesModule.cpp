#include "ScenesModule.h"
#include "Engine.h"
#include "EngineProfiler.h"
#include "Serializer.h"
#include "CameraManager.h"

namespace Glory
{
	ScenesModule::ScenesModule() : m_ActiveSceneIndex(0)
	{
	}
	ScenesModule::~ScenesModule()
	{
	}
	const std::type_info& ScenesModule::GetModuleType()
	{
		return typeid(ScenesModule);
	}

	GScene* ScenesModule::CreateEmptyScene(const std::string& name)
	{
		Profiler::BeginSample("ScenesModule::CreateEmptyScene");
		GScene* pScene = CreateScene(name);
		pScene->Initialize();
		m_pOpenScenes.push_back(pScene);
		Profiler::EndSample();
		return pScene;
	}

	size_t ScenesModule::OpenScenesCount()
	{
		return m_pOpenScenes.size();
	}

	GScene* ScenesModule::GetOpenScene(size_t index)
	{
		if (index >= m_pOpenScenes.size()) return nullptr;
		return m_pOpenScenes[index];
	}

	GScene* ScenesModule::GetOpenScene(UUID uuid)
	{
		auto it = std::find_if(m_pOpenScenes.begin(), m_pOpenScenes.end(), [&](GScene* pScene) {return pScene->GetUUID() == uuid; });
		if (it == m_pOpenScenes.end()) return nullptr;
		return *it;
	}

	GScene* ScenesModule::GetActiveScene()
	{
		if (m_ActiveSceneIndex >= m_pOpenScenes.size()) return nullptr;
		return m_pOpenScenes[m_ActiveSceneIndex];
	}

	void ScenesModule::SetActiveScene(GScene* pScene)
	{
		auto it = std::find(m_pOpenScenes.begin(), m_pOpenScenes.end(), pScene);
		if (it == m_pOpenScenes.end()) return;
		m_ActiveSceneIndex = it - m_pOpenScenes.begin();
	}

	void ScenesModule::CloseAllScenes()
	{
		Cleanup();
	}

	void ScenesModule::OpenScene(const std::string& path, UUID uuid)
	{
		YAML::Node node = YAML::LoadFile(path);
		std::filesystem::path filePath = path;
		GScene* pScene = Serializer::DeserializeObjectOfType<GScene>(node, filePath.filename().replace_extension().string());
		if (pScene == nullptr) return;
		pScene->SetUUID(uuid);
		m_pOpenScenes.push_back(pScene);
		OnSceneOpen(uuid);
	}

	void ScenesModule::AddOpenScene(GScene* pScene, UUID uuid)
	{
		if (pScene == nullptr) return;
		if (uuid) pScene->SetUUID(uuid);
		m_pOpenScenes.push_back(pScene);
		OnSceneOpen(uuid);
	}

	void ScenesModule::CloseScene(UUID uuid)
	{
		auto it = std::find_if(m_pOpenScenes.begin(), m_pOpenScenes.end(), [&](GScene* pScene) { return pScene->GetUUID() == uuid; });
		if (it == m_pOpenScenes.end()) return;
		size_t index = it - m_pOpenScenes.begin();
		GScene* pActiveScene = m_pOpenScenes[m_ActiveSceneIndex];

		OnSceneClose(uuid);
		GScene* pScene = *it;
		delete pScene;
		m_pOpenScenes.erase(it);

		if (index == m_ActiveSceneIndex || m_pOpenScenes.size() <= 0)
		{
			m_ActiveSceneIndex = 0;
			return;
		}

		it = std::find(m_pOpenScenes.begin(), m_pOpenScenes.end(), pActiveScene);
		m_ActiveSceneIndex = it - m_pOpenScenes.begin();
	}

	void ScenesModule::SetHoveringObject(uint64_t objectID)
	{
		std::unique_lock<std::mutex> lock(m_HoveringLock);
		m_pHoveringObject = GetSceneObjectFromObjectID(objectID);
		lock.unlock();
	}

	SceneObject* ScenesModule::GetHoveringObject()
	{
		SceneObject* pObject = nullptr;
		std::unique_lock<std::mutex> lock(m_HoveringLock);
		pObject = m_pHoveringObject;
		lock.unlock();
		return pObject;
	}

	void ScenesModule::Cleanup()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pOpenScenes.clear();
		m_ActiveSceneIndex = 0;
	}

	void ScenesModule::Update()
	{
		Profiler::BeginSample("ScenesModule::Tick");
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnTick(); });
		Profiler::EndSample();
	}

	void ScenesModule::Draw()
	{
		Profiler::BeginSample("ScenesModule::Paint");
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
		Profiler::EndSample();
	}

	SceneObject* ScenesModule::CreateObject(GScene* pScene, const std::string& name, UUID uuid)
	{
		return pScene->CreateObject(name, uuid);
	}
}
