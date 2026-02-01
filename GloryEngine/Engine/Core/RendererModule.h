#pragma once
#include "Module.h"
#include "RenderFrame.h"
#include "CameraRef.h"
#include "VertexHelpers.h"
#include "ShapeProperty.h"
#include "SceneObjectRef.h"
#include "GraphicsHandles.h"

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
	class GraphicsDevice;
	struct RenderPass;
	struct TextureCreateInfo;

	struct PickResult
	{
		const UUID m_CameraID;
		const SceneObjectRef m_Object;
		const glm::vec3 m_WorldPosition;
		const glm::vec3 m_Normal;
	};

	struct PerCameraData
	{
		glm::mat4 m_View;
		glm::mat4 m_Projection;
		glm::mat4 m_ViewInverse;
		glm::mat4 m_ProjectionInverse;
		float m_Near;
		float m_Far;
		glm::vec2 m_Resolution;
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

		void clear()
		{
			m_Data.clear();
			m_Dirty = true;
			m_DirtyRange.first = 0;
			m_DirtyRange.second = 1;
		}

		void resize(size_t newSize)
		{
			m_Data.resize(newSize);
			m_Dirty = true;
			m_DirtyRange.first = 0;
			m_DirtyRange.second = newSize;
			m_SizeDirty = true;
		}

		operator bool()
		{
			const bool value = m_Dirty;
			m_Dirty = false;
			return value;
		}

		bool SizeDirty()
		{
			const bool value = m_SizeDirty;
			m_SizeDirty = false;
			return value;
		}

		std::vector<T>* operator->() { return &m_Data; }

		void SetDirty(size_t index)
		{
			if (index >= m_Data.size()) return;

			const bool wasDirty = m_Dirty;
			m_Dirty = true;
			if (!wasDirty)
			{
				m_DirtyRange.first = index;
				m_DirtyRange.second = index + 1;
				return;
			}
			if (index < m_DirtyRange.first)
				m_DirtyRange.first = index;
			if (index + 1 > m_DirtyRange.second)
				m_DirtyRange.second = index + 1;
		}

		void SetDirty()
		{
			m_Dirty = true;
			m_DirtyRange.first = 0;
			m_DirtyRange.second = m_Data.size();
		}

		size_t DirtySize()
		{
			return m_DirtyRange.second - m_DirtyRange.first;
		}

		void* DirtyStart()
		{
			char* start = (char*)m_Data.data();
			return start + m_DirtyRange.first*sizeof(T);
		}

		size_t TotalByteSize()
		{
			return sizeof(T)*m_Data.size();
		}

		std::vector<T> m_Data;
		bool m_Dirty{ false };
		bool m_SizeDirty{ false };
		std::pair<size_t, size_t> m_DirtyRange{ 0, 0 };
	};

	struct PipelineMeshBatch
	{
		PipelineMeshBatch(UUID mesh) : m_Mesh(mesh) {}

		UUID m_Mesh;
		std::vector<glm::mat4> m_Worlds;
		std::vector<LayerMask> m_LayerMasks;
		std::vector<std::pair<UUID, UUID>> m_ObjectIDs;
		std::vector<uint32_t> m_MaterialIndices;
	};

	struct PipelineBatch
	{
		PipelineBatch(UUID pipeline);
		virtual ~PipelineBatch();
		
		void Reset();

		UUID m_PipelineID;
		std::unordered_map<UUID, PipelineMeshBatch> m_Meshes;
		std::vector<UUID> m_UniqueMeshOrder;
		std::vector<UUID> m_UniqueMaterials;
		bool m_Dirty;
	};

	struct PostProcess
	{
		std::string m_Name;
		int32_t m_Priority = -10;
		std::function<bool(GraphicsDevice*, CameraRef, size_t, CommandBufferHandle, size_t,
			RenderPassHandle, DescriptorSetHandle)> m_Callback;
	};

	class RendererModule : public Module
	{
	public:
		RendererModule();
		virtual ~RendererModule();

		virtual const std::type_info& GetModuleType() override;

		void SubmitStatic(RenderData&& renderData);
		void UpdateStatic(UUID pipelineID, UUID meshID, UUID objectID, glm::mat4 world);
		void UnsubmitStatic(UUID pipelineID, UUID meshID, UUID objectID);
		void SubmitDynamic(RenderData&& renderData);
		void SubmitLate(RenderData&& renderData);
		void SubmitCamera(CameraRef camera);
		void UnsubmitCamera(CameraRef camera);
		void UpdateCamera(CameraRef camera);
		size_t Submit(const glm::ivec2& pickPos, UUID cameraID);
		void Submit(LightData&& light, glm::mat4&& lightView, glm::mat4&& lightProjection, UUID id);

		virtual void OnBeginFrame() override;
		virtual void OnEndFrame() override;

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

		size_t CreateGPUTextureAtlas(TextureCreateInfo&& textureInfo, TextureHandle texture=0);
		GPUTextureAtlas& GetGPUTextureAtlas(size_t index);
		const GPUTextureAtlas& GetGPUTextureAtlas(size_t index) const;

		void Reset();

		virtual UUID TextPipelineID() const = 0;

		CameraRef GetActiveCamera(uint32_t cameraIndex) const;
		CameraRef GetOutputCamera(uint32_t cameraIndex) const;
		size_t GetOutputCameraCount() const;

		virtual size_t DefaultAttachmenmtIndex() const = 0;
		virtual size_t CameraAttachmentPreviewCount() const = 0;
		virtual std::string_view CameraAttachmentPreviewName(size_t index) const = 0;
		virtual TextureHandle CameraAttachmentPreview(CameraRef camera, size_t index) const = 0;
		virtual TextureHandle FinalColor() const = 0;
		virtual void VisualizeAttachment(CameraRef camera, size_t index) = 0;

		virtual size_t DebugOverlayCount() const = 0;
		virtual std::string_view DebugOverlayName(size_t index) const = 0;
		virtual void SetDebugOverlayEnabled(CameraRef camera, size_t index, bool enabled=true) = 0;
		virtual bool DebugOverlayEnabled(CameraRef camera, size_t index) const = 0;

		bool ResolutionChanged() const;
		const glm::uvec2& Resolution() const;

		virtual void PresentFrame() = 0;

		void SetSwapchain(SwapchainHandle swapchain);

		void SetEnabled(bool enabled=true);

		void InjectDatapass(std::function<void(GraphicsDevice*, RendererModule*)> datapassFunc);
		void InjectSwapchainSubpass(std::function<void(GraphicsDevice*, RenderPassHandle, CommandBufferHandle)> subpassFunc);
		void InjectPreRenderPass(std::function<void(GraphicsDevice*, CommandBufferHandle, uint32_t)> passFunc);

		void AddPostProcess(PostProcess&& postProcess);

		virtual uint32_t GetNumFramesInFlight() const = 0;
		virtual uint32_t GetCurrentFrameInFlight() const = 0;

		virtual RenderPassHandle GetSwapchainPass() const { return NULL; };
		virtual RenderPassHandle GetDummyPostProcessPass() const { return NULL; };

		void SetPipelineOrder(std::vector<UUID>&& pipelineOrder);

	protected:
		virtual void OnSubmitDynamic(const RenderData& renderData) {}
		virtual void OnSubmitCamera(CameraRef camera) {}
		virtual void OnUnsubmitCamera(CameraRef camera) {}
		virtual void OnCameraUpdated(CameraRef camera) {}
		virtual void OnSubmit(const LightData& light) {}
		virtual void OnWindowResized() {}
		virtual void OnSwapchainChanged() {}

	protected:
		virtual void Initialize() override;
		virtual void PostInitialize() override;
		virtual void Cleanup() = 0;

		virtual void OnPostInitialize() {};

		virtual void LoadSettings(ModuleSettings& settings) override;

		virtual void OnProcessData() override;

	protected:
		static const uint32_t MAX_LIGHTS = 3000;
		static const uint32_t MAX_CAMERAS = 100;

	protected:
		RenderFrame m_FrameData;
		size_t m_LastSubmittedObjectCount;
		size_t m_LastSubmittedCameraCount;

		uint32_t m_LineVertexCount;
		static const uint32_t MAX_LINE_VERTICES = 100000;
		LineVertex* m_pLineVertices;
		LineVertex* m_pLineVertex;

		std::mutex m_PickLock;
		std::vector<PickResult> m_PickResults;

		std::vector<GPUTextureAtlas> m_GPUTextureAtlases;

		std::vector<CameraRef> m_ActiveCameras;
		std::vector<CameraRef> m_OutputCameras;
		std::vector<RenderData> m_ToProcessStaticRenderData;
		std::vector<PipelineBatch> m_StaticPipelineRenderDatas;
		std::vector<PipelineBatch> m_DynamicPipelineRenderDatas;
		std::vector<PipelineBatch> m_DynamicLatePipelineRenderDatas;

		std::vector<PostProcess> m_PostProcesses;

		glm::uvec2 m_Resolution{ 1920, 1080 };
		glm::uvec2 m_LastResolution{ 1920, 1080 };

		SwapchainHandle m_Swapchain = 0;
		bool m_Enabled = true;

		std::vector<std::function<void(GraphicsDevice*, RendererModule*)>> m_InjectedDataPasses;
		std::vector<std::function<void(GraphicsDevice*, RenderPassHandle, CommandBufferHandle)>> m_InjectedSwapchainSubpasses;
		std::vector<std::function<void(GraphicsDevice*, CommandBufferHandle, uint32_t)>> m_InjectedPreRenderPasses;

		std::vector<UUID> m_PipelineOrder;
	};
}
