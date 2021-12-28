#include "ScenesModule.h"
#include "Engine.h"

namespace Glory
{
	const std::type_info& ScenesModule::GetModuleType()
	{
		return typeid(ScenesModule);
	}

	GScene* ScenesModule::CreateEmptyScene(const std::string& name)
	{
		GScene* pScene = CreateScene(name);
		pScene->Initialize();
		m_pOpenScenes.push_back(pScene);
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
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnTick(); });
	}

	void ScenesModule::Paint()
	{
		std::for_each(m_pOpenScenes.begin(), m_pOpenScenes.end(), [](GScene* pScene) { pScene->OnPaint(); });
	}

	SceneObject* ScenesModule::CreateObject(GScene* pScene, const std::string& name, UUID uuid)
	{
		return pScene->CreateObject(name, uuid);
	}
}
