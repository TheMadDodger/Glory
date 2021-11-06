#include "ScenesModule.h"
#include "Engine.h"

namespace Glory
{
	const std::type_info& ScenesModule::GetModuleType()
	{
		return typeid(ScenesModule);
	}

	GScene* ScenesModule::CreateEmptyScene()
	{
		GScene* pScene = CreateScene("New Scene");
		pScene->Initialize();
		m_pOpenScenes.push_back(pScene);
		return pScene;
	}

	void ScenesModule::Cleanup()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { delete pScene; });
	}

	void ScenesModule::Tick()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnTick(); });
	}

	void ScenesModule::Paint()
	{
		m_pEngine->GetRendererModule()->StartFrame();
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
		m_pEngine->GetRendererModule()->EndFrame();
	}
}
