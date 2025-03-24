#pragma once
#include "RenderTexture.h"

#include <vector>

namespace Glory
{
	class Engine;

	class DisplayManager
	{
	public:
		virtual ~DisplayManager();

		RenderTexture* GetDisplayRenderTexture(size_t index);
		static const int MAX_DISPLAYS = 10;

		void ClearAllDisplays(Engine* pEngine);
		void ResizeAllTextures(uint32_t width, uint32_t height);

		RenderTexture* CreateOutputTexture(Engine* pEngine, uint32_t width, uint32_t height);

		void SetResolution(uint32_t width, uint32_t height);
		void GetResolution(uint32_t& width, uint32_t& height);

	private:
		DisplayManager();

	private:
		void Initialize(Engine* pEngine);

	private:
		friend class Engine;
		friend class RendererModule;
		Engine* m_pEngine;
		bool m_Initialized;
		RenderTexture* m_pRenderTextures[MAX_DISPLAYS];

		uint32_t m_Width;
		uint32_t m_Height;
	};
}
