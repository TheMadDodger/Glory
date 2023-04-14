#pragma once
#include "Module.h"
#include "RenderFrame.h"
#include "CameraRef.h"
#include "LightData.h"
#include "VertexHelpers.h"
#include "ShapeProperty.h"

#include <vector>

namespace Glory
{
	class Buffer;
	class Mesh;

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

		virtual RenderTexture* CreateCameraRenderTexture(uint32_t width, uint32_t height);
		virtual void GetCameraRenderTextureAttachments(std::vector<Attachment>& atachments);
		virtual void OnCameraResize(CameraRef camera);
		virtual void OnCameraPerspectiveChanged(CameraRef camera);

		size_t LastSubmittedObjectCount();
		size_t LastSubmittedCameraCount();

		void SetNextFramePick(const glm::ivec2& coord, CameraRef camera);

		enum CircleUp
		{
			x,
			y,
			z
		};

		void DrawLine(const glm::mat4& transform, const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color);
		void DrawLineQuad(const glm::mat4& transform, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec4& color);
		void DrawLineCircle(const glm::mat4& transform, const glm::vec3& position, float radius, CircleUp up, const glm::vec4& color);
		void DrawLineBox(const glm::mat4& transform, const glm::vec3& position, const glm::vec3& extends, const glm::vec4& color);
		void DrawLineSphere(const glm::mat4& transform, const glm::vec3& position, float radius, const glm::vec4& color);

		void DrawLineShape(const glm::mat4& transform, const glm::vec3& position, const ShapeProperty& shape, const glm::vec4& color);

	protected:
		virtual void OnSubmit(const RenderData& renderData) {}
		virtual void OnSubmit(CameraRef camera) {}
		virtual void OnSubmit(const PointLight& light) {}

	protected:
		friend class GraphicsThread;
		virtual void Initialize() override;
		virtual void Cleanup() = 0;
		virtual void OnRender(CameraRef camera, const RenderData& renderData, const std::vector<PointLight>& lights = std::vector<PointLight>()) = 0;
		virtual void OnDoScreenRender(CameraRef camera, const FrameData<PointLight>& lights, uint32_t width, uint32_t height, RenderTexture* pRenderTexture) = 0;

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
		void CreateLineBuffer();
		void RenderLines(CameraRef camera);

	private:
		RenderFrame m_CurrentPreparingFrame;
		size_t m_LastSubmittedObjectCount;
		size_t m_LastSubmittedCameraCount;
		glm::ivec2 m_PickPos;
		CameraRef m_PickCamera;

		uint32_t m_LineVertexCount;
		Buffer* m_pLineBuffer;
		Mesh* m_pLineMesh;
		MaterialData* m_pLinesMaterialData;
		Material* m_pLinesMaterial;

		static const uint32_t MAX_LINE_VERTICES = 1000;
		LineVertex* m_pLineVertices;
		LineVertex* m_pLineVertex;
	};
}
