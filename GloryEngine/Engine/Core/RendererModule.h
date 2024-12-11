#pragma once
#include "Module.h"
#include "RenderFrame.h"
#include "CameraRef.h"
#include "LightData.h"
#include "VertexHelpers.h"
#include "ShapeProperty.h"
#include "SceneObjectRef.h"

#include <functional>
#include <vector>

namespace Glory
{
	class Buffer;
	class Mesh;
	class PipelineData;

	struct PickResult
	{
		const UUID m_CameraID;
		const SceneObjectRef m_Object;
		const glm::vec3 m_WorldPosition;
		const glm::vec3 m_Normal;
	};

	class RendererModule : public Module
	{
	public:
		RendererModule();
		virtual ~RendererModule();

		virtual const std::type_info& GetModuleType() override;

		void Submit(RenderData&& renderData);
		void Submit(TextRenderData&& renderData);
		void SubmitLate(RenderData&& renderData);
		void Submit(CameraRef camera);
		size_t Submit(const glm::ivec2& pickPos, UUID cameraID);
		void Submit(CameraRef camera, RenderTexture* pTexture);
		void Submit(PointLight&& light);

		void OnGameThreadFrameStart();
		void OnGameThreadFrameEnd();

		virtual void CreateCameraRenderTextures(uint32_t width, uint32_t height, std::vector<RenderTexture*>& renderTextures);
		virtual void GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos);
		virtual void OnCameraResize(CameraRef camera);
		virtual void OnCameraPerspectiveChanged(CameraRef camera);

		size_t LastSubmittedObjectCount();
		size_t LastSubmittedCameraCount();

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

		bool PickResultValid(size_t index) const;
		bool PickResultIndex(UUID cameraID, size_t& index) const;
		const PickResult& GetPickResult(size_t index) const;
		void GetPickResult(UUID cameraID, std::function<void(const PickResult&)> callback);

	protected:
		virtual void OnSubmit(const RenderData& renderData) {}
		virtual void OnSubmit(const TextRenderData& renderData) {}
		virtual void OnSubmit(CameraRef camera) {}
		virtual void OnSubmit(const PointLight& light) {}

	protected:
		friend class GraphicsThread;
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Cleanup() = 0;
		virtual void OnRender(CameraRef camera, const RenderData& renderData, const std::vector<PointLight>& lights = std::vector<PointLight>()) = 0;
		virtual void OnRender(CameraRef camera, const TextRenderData& renderData, const std::vector<PointLight>& lights = std::vector<PointLight>()) = 0;
		virtual void OnRenderEffects(CameraRef camera, RenderTexture* pRenderTexture) = 0;
		virtual void OnDoScreenRender(CameraRef camera, const FrameData<PointLight>& lights, uint32_t width, uint32_t height, RenderTexture* pRenderTexture) = 0;

		virtual void OnInitialize() {};
		virtual void OnPostInitialize() {};
		virtual void OnThreadedInitialize() {}
		virtual void OnThreadedCleanup() {}

		virtual void OnStartCameraRender(CameraRef camera, const FrameData<PointLight>& lights) = 0;
		virtual void OnEndCameraRender(CameraRef camera, const FrameData<PointLight>& lights) = 0;

		virtual void Draw() override;

	protected:
		static const uint32_t MAX_LIGHTS = 3000;

	private:
		// Run on Graphics Thread
		void ThreadedInitialize();
		void ThreadedCleanup();
		void Render(const RenderFrame& frame);
		void DoPicking(const glm::ivec2& pos, CameraRef camera);
		void CreateLineBuffer();
		void RenderLines(CameraRef camera);

	private:
		RenderFrame m_CurrentPreparingFrame;
		size_t m_LastSubmittedObjectCount;
		size_t m_LastSubmittedCameraCount;

		uint32_t m_LineVertexCount;
		Buffer* m_pLineBuffer;
		Mesh* m_pLineMesh;
		PipelineData* m_pLinesPipelineData;
		MaterialData* m_pLinesMaterialData;
		Material* m_pLinesMaterial;

		static const uint32_t MAX_LINE_VERTICES = 1000;
		LineVertex* m_pLineVertices;
		LineVertex* m_pLineVertex;

		std::mutex m_PickLock;
		std::vector<PickResult> m_LastFramePickResults;
		std::vector<PickResult> m_PickResults;
	};
}
