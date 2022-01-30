#pragma once
#include "Module.h"
#include "RenderFrame.h"
#include <vector>
#include "CameraRef.h"
#include "LightData.h"

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
		void Submit(CameraRef camera, RenderTexture* pTexture);
		void Submit(const PointLight& light);

		void OnGameThreadFrameStart();
		void OnGameThreadFrameEnd();

		virtual RenderTexture* CreateCameraRenderTexture(size_t width, size_t height);
		virtual void OnCameraResize(CameraRef camera);

	protected:
		virtual void OnSubmit(const RenderData& renderData) {}
		virtual void OnSubmit(CameraRef camera) {}
		virtual void OnSubmit(const PointLight& light) {}

	protected:
		friend class GraphicsThread;
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void OnRender(CameraRef camera, const RenderData& renderData, const std::vector<PointLight>& lights = std::vector<PointLight>()) = 0;
		virtual void OnDoScreenRender(CameraRef camera, const std::vector<PointLight>& lights, size_t width, size_t height, RenderTexture* pRenderTexture) = 0;

		virtual void OnThreadedInitialize() {}
		virtual void OnThreadedCleanup() {}

		virtual void OnStartCameraRender(CameraRef camera, const std::vector<PointLight>& lights) = 0;
		virtual void OnEndCameraRender(CameraRef camera, const std::vector<PointLight>& lights) = 0;

	protected:
		static const uint32_t MAX_LIGHTS = 3000;

	private:
		// Run on Graphics Thread
		void ThreadedInitialize();
		void ThreadedCleanup();
		void Render(const RenderFrame& frame);

	private:
		RenderFrame m_CurrentPreparingFrame;
	};
}
