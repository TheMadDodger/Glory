#include "DisplayManager.h"
#include "EngineProfiler.h"
#include "WindowModule.h"
#include "GraphicsModule.h"
#include "Engine.h"

namespace Glory
{
	RenderTexture* DisplayManager::GetDisplayRenderTexture(size_t index)
	{
		return index >= MAX_DISPLAYS ? nullptr : m_pRenderTextures[index];
	}

	void DisplayManager::ClearAllDisplays(Engine* pEngine)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "DisplayManager::ClearAllDisplays" };
		for (size_t i = 0; i < MAX_DISPLAYS; i++)
		{
			m_pRenderTextures[i]->BindForDraw();
			pEngine->GetMainModule<GraphicsModule>()->Clear();
			m_pRenderTextures[i]->UnBindForDraw();
		}
	}

	void DisplayManager::ResizeAllTextures(uint32_t width, uint32_t height)
	{
		for (size_t i = 0; i < MAX_DISPLAYS; i++)
		{
			m_pRenderTextures[i]->Resize(width, height);
		}
	}

	RenderTexture* DisplayManager::CreateOutputTexture(Engine* pEngine, uint32_t width, uint32_t height)
	{
		RenderTextureCreateInfo createInfo(width, height, false);
		createInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));
		return pEngine->GetMainModule<GraphicsModule>()->GetResourceManager()->CreateRenderTexture(createInfo);
	}

	void DisplayManager::SetResolution(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0) return;
		if (width == m_Width && height == m_Height) return;
		m_Width = width;
		m_Height = height;
		if (!m_Initialized) return;
		ResizeAllTextures(m_Width, m_Height);
	}

	void DisplayManager::GetResolution(uint32_t& width, uint32_t& height)
	{
		width = m_Width;
		height = m_Height;
	}
	
	DisplayManager::DisplayManager() : m_pEngine(nullptr), m_Initialized(false),
		m_pRenderTextures(), m_Width(1920), m_Height(1080)
	{
	}

	DisplayManager::~DisplayManager()
	{
		m_Initialized = false;
	}

	void DisplayManager::Initialize(Engine* pEngine)
	{
		m_pEngine = pEngine;

		RenderTextureCreateInfo createInfo(m_Width, m_Height, false);
		createInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color));

		for (size_t i = 0; i < MAX_DISPLAYS; i++)
		{
			m_pRenderTextures[i] = pEngine->GetMainModule<GraphicsModule>()->GetResourceManager()->CreateRenderTexture(createInfo);
			m_pRenderTextures[i]->BindForDraw();
			pEngine->GetMainModule<GraphicsModule>()->Clear();
			m_pRenderTextures[i]->UnBindForDraw();
		}

		m_Initialized = true;
	}
}
