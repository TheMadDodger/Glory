#include "ScenesModule.h"
#include "Engine.h"
#include "EngineProfiler.h"

namespace Glory
{
	const std::type_info& ScenesModule::GetModuleType()
	{
		return typeid(ScenesModule);
	}

	GScene* ScenesModule::CreateEmptyScene()
	{
		Profiler::BeginSample("ScenesModule::CreateEmptyScene");
		GScene* pScene = CreateScene("New Scene");
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

	GScene* ScenesModule::GetActiveScene()
	{
		if (m_ActiveSceneIndex >= m_pOpenScenes.size()) return nullptr;
		return m_pOpenScenes[m_ActiveSceneIndex];
	}

	void ScenesModule::Cleanup()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
	}

	void ScenesModule::Tick()
	{
		Profiler::BeginSample("ScenesModule::Tick");
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnTick(); });
		Profiler::EndSample();
	}

	void ScenesModule::Paint()
	{
		Profiler::BeginSample("ScenesModule::Paint");
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
		Profiler::EndSample();
	}
}
