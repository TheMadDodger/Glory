#pragma once
#include "Module.h"
#include <vector>
#include "RenderFrame.h"

namespace Glory
{
	class RendererModule : public Module
	{
	public:
		RendererModule();
		virtual ~RendererModule();

		virtual const std::type_info& GetModuleType() override;

		void Submit(const RenderData& renderData);
		void Submit(Camera* pCamera);

		void StartFrame();
		void EndFrame();

	protected:
		virtual void OnSubmit(const RenderData& renderData) {}
		virtual void OnSubmit(Camera* pCamera) {}

	protected:
		friend class GraphicsThread;
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void OnRender(Camera* pCamera, const RenderData& renderData) = 0;
		// Temporary
		virtual void OnFinalRender(RenderTexture* pRenderTexture) = 0;
		
	private:
		void ThreadedInitialize();
		void ThreadedCleanup();
		void Render(const RenderFrame& frame);

	private:
		RenderFrame m_CurrentPreparingFrame;
	};
}
