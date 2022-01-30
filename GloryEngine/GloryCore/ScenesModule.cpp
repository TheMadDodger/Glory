#include "ScenesModule.h"
#include "Engine.h"
#include "EngineProfiler.h"
#include "Serializer.h"

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
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		m_pOpenScenes.push_back(pScene);
		lock.unlock();
		Profiler::EndSample();
		return pScene;
	}

	size_t ScenesModule::OpenScenesCount()
	{
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		return m_pOpenScenes.size();
	}

	GScene* ScenesModule::GetOpenScene(size_t index)
	{
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		if (index >= m_pOpenScenes.size()) return nullptr;
		return m_pOpenScenes[index];
	}

	GScene* ScenesModule::GetOpenScene(UUID uuid)
	{
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		auto it = std::find_if(m_pOpenScenes.begin(), m_pOpenScenes.end(), [&](GScene* pScene) {return pScene->GetUUID() == uuid; });
		if (it == m_pOpenScenes.end()) return nullptr;
		return *it;
	}

	GScene* ScenesModule::GetActiveScene()
	{
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		if (m_ActiveSceneIndex >= m_pOpenScenes.size()) return nullptr;
		return m_pOpenScenes[m_ActiveSceneIndex];
	}

	void ScenesModule::CloseAllScenes()
	{
		Cleanup();
	}

	void ScenesModule::OpenScene(const std::string& path, UUID uuid)
	{
		YAML::Node node = YAML::LoadFile(path);
		GScene* pScene = (GScene*)Serializer::DeserializeObjectOfType<GScene>(node);
		pScene->SetUUID(uuid);
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		m_pOpenScenes.push_back(pScene);
		lock.unlock();
	}

	void ScenesModule::CloseScene(UUID uuid)
	{
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		auto it = std::find_if(m_pOpenScenes.begin(), m_pOpenScenes.end(), [&](GScene* pScene) { return pScene->GetUUID() == uuid; });
		if (it == m_pOpenScenes.end()) return;
		size_t index = it - m_pOpenScenes.begin();
		GScene* pActiveScene = m_pOpenScenes[m_ActiveSceneIndex];

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

	void ScenesModule::Cleanup()
	{
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
		m_pOpenScenes.clear();
		m_ActiveSceneIndex = 0;
		lock.unlock();
	}

	void ScenesModule::Update()
	{
		Profiler::BeginSample("ScenesModule::Tick");
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnTick(); });
		lock.unlock();
		Profiler::EndSample();
	}

	void ScenesModule::Draw()
	{
		Profiler::BeginSample("ScenesModule::Paint");
		std::unique_lock<std::mutex> lock(m_OpenScenesLock);
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
		lock.unlock();
		Profiler::EndSample();
	}

	SceneObject* ScenesModule::CreateObject(GScene* pScene, const std::string& name, UUID uuid)
	{
		return pScene->CreateObject(name, uuid);
	}
}
