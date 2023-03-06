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
		virtual void GetCameraRenderTextureAttachments(std::vector<Attachment>& atachments);
		virtual void OnCameraResize(CameraRef camera);

		size_t LastSubmittedObjectCount();
		size_t LastSubmittedCameraCount();

		void SetNextFramePick(const glm::ivec2& coord, CameraRef camera);

	protected:
		virtual void OnSubmit(const RenderData& renderData) {}
		virtual void OnSubmit(CameraRef camera) {}
		virtual void OnSubmit(const PointLight& light) {}

	protected:
		friend class GraphicsThread;
		virtual void Initialize() override;
		virtual void Cleanup() = 0;
		virtual void OnRender(CameraRef camera, const RenderData& renderData, const std::vector<PointLight>& lights = std::vector<PointLight>()) = 0;
		virtual void OnDoScreenRender(CameraRef camera, const FrameData<PointLight>& lights, size_t width, size_t height, RenderTexture* pRenderTexture) = 0;

		virtual void OnInitialize() {};
		virtual void OnThreadedInitialize() {}
		virtual void OnThreadedCleanup() {}

		virtual void OnStartCameraRender(CameraRef camera, const FrameData<PointLight>& lights) = 0;
		virtual void OnEndCameraRender(CameraRef camera, const FrameData<PointLight>& lights) = 0;

	protected:
		static const uint32_t MAX_LIGHTS = 3000;

	private:
		// Run on Graphics Thread
		void ThreadedInitialize();
		void ThreadedCleanup();
		void Render(const RenderFrame& frame);

		void ReadHoveringObject();

	private:
		RenderFrame m_CurrentPreparingFrame;
		size_t m_LastSubmittedObjectCount;
		size_t m_LastSubmittedCameraCount;
		glm::ivec2 m_PickPos;
		CameraRef m_PickCamera;
	};
}
