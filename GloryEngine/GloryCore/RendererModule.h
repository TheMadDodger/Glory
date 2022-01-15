#pragma once
#include "Module.h"
#include "RenderFrame.h"
#include <vector>
#include "CameraRef.h"

namespace Glory
{
	class RendererModule : public Module
	{
	public:
		RendererModule();
		virtual ~RendererModule();

		virtual const std::type_info& GetModuleType() override;

		void Submit(const RenderData& renderData);
		void Submit(CameraRef camera);

		void StartFrame();
		void EndFrame();

	protected:
		virtual void OnSubmit(const RenderData& renderData) {}
		virtual void OnSubmit(CameraRef camera) {}

	protected:
		friend class GraphicsThread;
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void OnRender(CameraRef camera, const RenderData& renderData) = 0;
		virtual void OnDoScreenRender(RenderTexture* pRenderTexture) = 0;

		virtual void OnThreadedInitialize() {}
		virtual void OnThreadedCleanup() {}
		
	private:
		void ThreadedInitialize();
		void ThreadedCleanup();
		void Render(const RenderFrame& frame);

	private:
		RenderFrame m_CurrentPreparingFrame;
	};
}
