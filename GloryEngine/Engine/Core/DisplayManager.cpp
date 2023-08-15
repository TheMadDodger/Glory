#include "DisplayManager.h"
#include "EngineProfiler.h"
#include "GloryContext.h"
#include "WindowModule.h"
#include "GraphicsModule.h"

namespace Glory
{
	RenderTexture* DisplayManager::GetDisplayRenderTexture(size_t index)
	{
		return index >= MAX_DISPLAYS ? nullptr : DISPLAYMANAGR->m_pRenderTextures[index];
	}

	void DisplayManager::ClearAllDisplays(Engine* pEngine)
	{
		Profiler::BeginSample("DisplayManager::ClearAllDisplays");
		for (size_t i = 0; i < MAX_DISPLAYS; i++)
		{
			DISPLAYMANAGR->m_pRenderTextures[i]->Bind();
			pEngine->GetMainModule<GraphicsModule>()->Clear();
			DISPLAYMANAGR->m_pRenderTextures[i]->UnBind();
		}
		Profiler::EndSample();
	}

	RenderTexture* DisplayManager::CreateOutputTexture(Engine* pEngine, uint32_t width, uint32_t height)
	{
		RenderTextureCreateInfo createInfo(width, height, false);
		createInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
		return pEngine->GetMainModule<GraphicsModule>()->GetResourceManager()->CreateRenderTexture(createInfo);
	}
	
	DisplayManager::DisplayManager() : m_pRenderTextures()
	{
	}

	DisplayManager::~DisplayManager()
	{
	}

	void DisplayManager::Initialize(Engine* pEngine)
	{
		int width, height;
		pEngine->GetMainModule<WindowModule>()->GetMainWindow()->GetDrawableSize(&width, &height);

		RenderTextureCreateInfo createInfo(width, height, false);
		createInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));

		for (size_t i = 0; i < MAX_DISPLAYS; i++)
		{
			DISPLAYMANAGR->m_pRenderTextures[i] = pEngine->GetMainModule<GraphicsModule>()->GetResourceManager()->CreateRenderTexture(createInfo);
			DISPLAYMANAGR->m_pRenderTextures[i]->Bind();
			pEngine->GetMainModule<GraphicsModule>()->Clear();
			DISPLAYMANAGR->m_pRenderTextures[i]->UnBind();
		}
	}
}
