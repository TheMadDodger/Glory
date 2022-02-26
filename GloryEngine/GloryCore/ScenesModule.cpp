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
		GScene* pScene = (GScene*)Serializer::DeserializeObjectOfType<GScene>(node);
		if (pScene == nullptr) return;
		pScene->SetUUID(uuid);
		m_pOpenScenes.push_back(pScene);
	}

	void ScenesModule::CloseScene(UUID uuid)
	{
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
