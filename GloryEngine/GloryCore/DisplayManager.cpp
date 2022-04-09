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

	RenderTexture* DisplayManager::CreateOutputTexture(Engine* pEngine, size_t width, size_t height)
	{
		RenderTextureCreateInfo createInfo(width, height, false);
		createInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
		return pEngine->GetGraphicsModule()->GetResourceManager()->CreateRenderTexture(createInfo);
	}
	
	DisplayManager::~DisplayManager()
	{
	}

	void DisplayManager::Initialize(Engine* pEngine)
	{
		int width, height;
		pEngine->GetWindowModule()->GetMainWindow()->GetDrawableSize(&width, &height);

		RenderTextureCreateInfo createInfo(width, height, false);
		createInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));

		for (size_t i = 0; i < MAX_DISPLAYS; i++)
		{
			m_pRenderTextures[i] = pEngine->GetGraphicsModule()->GetResourceManager()->CreateRenderTexture(createInfo);
			m_pRenderTextures[i]->Bind();
			pEngine->GetGraphicsModule()->Clear();
			m_pRenderTextures[i]->UnBind();
		}
	}
}
