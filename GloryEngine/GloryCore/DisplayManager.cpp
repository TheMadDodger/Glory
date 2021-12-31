#include "DisplayManager.h"
#include "EngineProfiler.h"

namespace Glory
{
	RenderTexture* DisplayManager::m_pRenderTextures[MAX_DISPLAYS];

	RenderTexture* DisplayManager::GetDisplayRenderTexture(size_t index)
	{
		return index >= MAX_DISPLAYS ? nullptr : m_pRenderTextures[index];
	}

	void DisplayManager::ClearAllDisplays(Engine* pEngine)
	{
		Profiler::BeginSample("DisplayManager::ClearAllDisplays");
		for (size_t i = 0; i < MAX_DISPLAYS; i++)
		{
			m_pRenderTextures[i]->Bind();
			pEngine->GetGraphicsModule()->Clear();
			m_pRenderTextures[i]->UnBind();
		}
		Profiler::EndSample();
	}
	
	DisplayManager::~DisplayManager()
	{
	}

	void DisplayManager::Initialize(Engine* pEngine)
	{
		int width, height;
		pEngine->GetWindowModule()->GetMainWindow()->GetDrawableSize(&width, &height);

		for (size_t i = 0; i < MAX_DISPLAYS; i++)
		{
			m_pRenderTextures[i] = pEngine->GetGraphicsModule()->GetResourceManager()->CreateRenderTexture(width, height, false);

			m_pRenderTextures[i]->Bind();
			pEngine->GetGraphicsModule()->Clear();
			m_pRenderTextures[i]->UnBind();
		}
	}
}
