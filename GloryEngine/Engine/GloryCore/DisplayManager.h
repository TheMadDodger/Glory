#pragma once
#include "RenderTexture.h"
#include <vector>
#include "Engine.h"

#define DISPLAYMANAGR Glory::GloryContext::GetDisplayManager()

namespace Glory
{
	class DisplayManager
	{
	public:
		static RenderTexture* GetDisplayRenderTexture(size_t index);
		static const int MAX_DISPLAYS = 10;

		static void ClearAllDisplays(Engine* pEngine);

		static RenderTexture* CreateOutputTexture(Engine* pEngine, uint32_t width, uint32_t height);

	private:
		DisplayManager();
		virtual ~DisplayManager();

	private:
		static void Initialize(Engine* pEngine);

	private:
		friend class RendererModule;
		friend class GloryContext;
		RenderTexture* m_pRenderTextures[MAX_DISPLAYS];
	};
}
