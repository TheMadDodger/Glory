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
#include <string_view>

namespace Glory
{
	class Buffer;
	class Mesh;
	class PipelineData;
	class CubemapData;
	class MaterialData;
	class GPUTextureAtlas;
	struct RenderPass;

	struct PickResult
	{
		const UUID m_CameraID;
		const SceneObjectRef m_Object;
		const glm::vec3 m_WorldPosition;
		const glm::vec3 m_Normal;
	};

	struct PerObjectData
	{
		UUID m_SceneID;
		UUID m_ObjectID;
		glm::mat4 m_World;
	};

	template<typename T>
	struct CPUBuffer
	{
		template <class... _Valty>
		decltype(auto) emplace_back(_Valty&&... _Val)
		{
			T& newElement = m_Data.emplace_back(std::forward<_Valty>(_Val)...);
			m_Dirty = true;
			return newElement;
		}

		operator bool()
		{
			const bool value = m_Dirty;
			m_Dirty = false;
			return value;
		}
		std::vector<T>* operator->() { return &m_Data; }
		std::vector<T> m_Data;
		bool m_Dirty{ false };
	};

	struct PipelineRenderData
	{
		PipelineRenderData(UUID pipeline);
		virtual ~PipelineRenderData();

		UUID m_Pipeline;
		CPUBuffer<UUID> m_Meshes;
		CPUBuffer<UUID> m_Materials;
		CPUBuffer<PerObjectData> m_PerObjectData;
		MeshData* m_pCombinedMesh;
		CPUBuffer<DrawElementsIndirectCommand> m_IndirectDrawCommands;
		bool m_Dirty;

		Buffer* m_pIndirectDrawCommandsBuffer;
		Buffer* m_pIndirectDrawPerObjectDataBuffer;
		Mesh* m_pIndirectDrawMesh;
	};

	class RendererModule : public Module
	{
	public:
		RendererModule();
		virtual ~RendererModule();

		virtual const std::type_info& GetModuleType() override;

		void SubmitStatic(RenderData&& renderData);
		void UpdateStatic(UUID pipelineID, UUID objectID, glm::mat4 world);
		void UnsubmitStatic(UUID pipelineID, UUID objectID);
		void SubmitDynamic(RenderData&& renderData);
		void SubmitDynamic(TextRenderData&& renderData);
		void SubmitLate(RenderData&& renderData);
		void Submit(CameraRef camera);
		size_t Submit(const glm::ivec2& pickPos, UUID cameraID);
		void Submit(CameraRef camera, RenderTexture* pTexture);
		void Submit(LightData&& light, glm::mat4&& lightSpace, UUID id);

		virtual void OnBeginFrame() override;

		virtual void CreateCameraRenderTextures(uint32_t width, uint32_t height, std::vector<RenderTexture*>& renderTextures);
		virtual void GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos);
		virtual void OnCameraResize(CameraRef camera);
		virtual void OnCameraPerspectiveChanged(CameraRef camera);
		virtual MaterialData* GetInternalMaterial(std::string_view name) const = 0;

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

		void OnWindowResize(glm::uvec2 size);

		void AddRenderPass(RenderPassType type, RenderPass&& pass);
		void RemoveRenderPass(RenderPassType type, std::string_view name);

		void RenderObject(CameraRef camera, const RenderData& renderData);

		void RenderOnBackBuffer(RenderTexture* pTexture);

		GPUTextureAtlas* CreateGPUTextureAtlas(TextureCreateInfo&& textureInfo, bool depth=false);

		void Reset();

	protected:
		virtual void OnSubmitDynamic(const RenderData& renderData) {}
		virtual void OnSubmitDynamic(const TextRenderData& renderData) {}
		virtual void OnSubmit(CameraRef camera) {}
		virtual void OnSubmit(const LightData& light) {}

	protected:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Cleanup() = 0;
		virtual void OnRender(CameraRef camera, const RenderData& renderData, const std::vector<LightData>& lights = std::vector<LightData>()) = 0;
		virtual void OnRender(CameraRef camera, const TextRenderData& renderData, const std::vector<LightData>& lights = std::vector<LightData>()) = 0;
		virtual void OnRenderEffects(CameraRef camera, RenderTexture* pRenderTexture) = 0;
		virtual void OnRenderSkybox(CameraRef camera, CubemapData* pCubemap) = 0;
		virtual void OnDoCompositing(CameraRef camera, uint32_t width, uint32_t height, RenderTexture* pRenderTexture) = 0;
		virtual void OnDisplayCopy(RenderTexture* pRenderTexture, uint32_t width, uint32_t height) = 0;

		virtual void OnPostInitialize() {};

		virtual void OnStartCameraRender(CameraRef camera, const FrameData<LightData>& lights) = 0;
		virtual void OnEndCameraRender(CameraRef camera, const FrameData<LightData>& lights) = 0;

		virtual void Draw() override;

		virtual void LoadSettings(ModuleSettings& settings) override;

	protected:
		static const uint32_t MAX_LIGHTS = 3000;

	private:
		// Run on Graphics Thread
		void Render();
		void DoPicking(const glm::ivec2& pos, CameraRef camera);
		void CreateLineBuffer();
		void RenderLines(CameraRef camera);

		void MainObjectPass(CameraRef camera, const RenderFrame& frame);
		void SkyboxPass(CameraRef camera, const RenderFrame& frame);
		void MainTextPass(CameraRef camera, const RenderFrame& frame);
		void MainLateObjectPass(CameraRef camera, const RenderFrame& frame);
		void DeferredCompositePass(CameraRef camera, const RenderFrame& frame);

	protected:
		RenderFrame m_FrameData;
		size_t m_LastSubmittedObjectCount;
		size_t m_LastSubmittedCameraCount;

		uint32_t m_LineVertexCount;
		Buffer* m_pLineBuffer;
		Mesh* m_pLineMesh;
		MaterialData* m_pLinesMaterialData;

		static const uint32_t MAX_LINE_VERTICES = 100000;
		LineVertex* m_pLineVertices;
		LineVertex* m_pLineVertex;

		std::mutex m_PickLock;
		std::vector<PickResult> m_LastFramePickResults;
		std::vector<PickResult> m_PickResults;

		std::atomic_bool m_DisplaysDirty;

		std::vector<std::vector<RenderPass>> m_RenderPasses;

		std::vector<GPUTextureAtlas> m_GPUTextureAtlases;

		std::vector<RenderData> m_ToProcessStaticRenderData;
		std::vector<PipelineRenderData> m_StaticPipelineRenderDatas;
		std::vector<PipelineRenderData> m_DynamicPipelineRenderDatas;
	};
}
