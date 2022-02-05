#pragma once
#include "RenderTexture.h"
#include <vector>
#include "Engine.h"

namespace Glory
{
	class DisplayManager
	{
	public:
		static RenderTexture* GetDisplayRenderTexture(size_t index);
		static const int MAX_DISPLAYS = 10;

		static void ClearAllDisplays(Engine* pEngine);

		static RenderTexture* CreateOutputTexture(Engine* pEngine, size_t width, size_t height);

	private:
		DisplayManager();
		virtual ~DisplayManager();

	private:
		static void Initialize(Engine* pEngine);

	private:
		friend class RendererModule;
		static RenderTexture* m_pRenderTextures[MAX_DISPLAYS];
	};
}
