#pragma once
#include <cstdint>
#include <vector>
#include <string_view>

#include <glm/glm.hpp>

#include <GraphicsHandles.h>
#include <BitSet.h>

namespace Glory
{
	struct VolumeTileAABB
	{
		glm::vec4 MinPoint;
		glm::vec4 MaxPoint;
	};

	struct LightGrid
	{
		uint32_t Offset;
		uint32_t Count;
	};

	struct ClusterConstants
	{
		glm::uvec4 GridSize;
		uint32_t LightCount;
		uint32_t CameraIndex;
	};

	struct SSAOConstants
	{
		uint32_t CameraIndex;
		int32_t KernelSize;
		float SampleRadius;
		float SampleBias;
	};

	struct RenderConstants
	{
		glm::uvec4 m_GridSize;
		UUID m_SceneID;
		UUID m_ObjectID;
		uint32_t m_ObjectDataIndex;
		uint32_t m_CameraIndex;
		uint32_t m_MaterialIndex;
		uint32_t m_LightCount;
	};

	struct PickingConstants
	{
		uint32_t m_CameraIndex;
		uint32_t m_NumPickings;
		uint32_t m_Padding1;
		uint32_t m_Padding2;
		glm::ivec2 m_Picks[8];
	};

	struct BufferBindingIndices
	{
		static constexpr uint32_t RenderConstants = 0;
		static constexpr uint32_t CameraDatas = 1;
		static constexpr uint32_t WorldTransforms = 2;
		static constexpr uint32_t LightDatas = 3;
		static constexpr uint32_t LightSpaceTransforms = 4;
		static constexpr uint32_t LightIndices = 5;
		static constexpr uint32_t LightGrid = 6;
		static constexpr uint32_t LightDistances = 7;
		static constexpr uint32_t PickingResults = 2;

		static constexpr uint32_t Materials = 8;
		static constexpr uint32_t HasTexture = 9;

		static constexpr uint32_t Clusters = 2;
		static constexpr uint32_t SampleDome = 2;
	};

	struct LightComplexityConstants
	{
		glm::uvec4 GridSize;
		glm::uvec2 Resolution;
		float zNear;
		float zFar;
	};

	struct OrderedObject
	{
		float minDistance;
		UUID meshID;
		uint32_t meshObjectIndex;
		uint32_t objectIndex;

		bool operator>(const OrderedObject& other) const
		{
			return minDistance < other.minDistance;
		}

		bool operator<(const OrderedObject& other) const
		{
			return minDistance > other.minDistance;
		}

		bool operator==(const OrderedObject& other) const
		{
			return minDistance == other.minDistance;
		}

		bool operator!=(const OrderedObject& other) const
		{
			return minDistance != other.minDistance;
		}

		bool operator>=(const OrderedObject& other) const
		{
			return minDistance <= other.minDistance;
		}

		bool operator<=(const OrderedObject& other) const
		{
			return minDistance >= other.minDistance;
		}
	};

	struct GPUPickResult
	{
		UUID SceneID;
		UUID ObjectID;
		glm::vec4 Normal;
		glm::vec4 Position;
	};

	struct PostProcessPass
	{
		void Swap()
		{
			RenderPassHandle temp = m_FrontBufferPass;
			m_FrontBufferPass = m_BackBufferPass;
			m_BackBufferPass = temp;
			DescriptorSetHandle tempDs = m_FrontDescriptor;
			m_FrontDescriptor = m_BackDescriptor;
			m_BackDescriptor = tempDs;
		}

		RenderPassHandle m_FrontBufferPass = 0;
		DescriptorSetHandle m_FrontDescriptor = 0;
		RenderPassHandle m_BackBufferPass = 0;
		DescriptorSetHandle m_BackDescriptor = 0;
	};

	enum CameraAttachment
	{
		ObjectID,
		Color,
		Normal,
		AO,
		Depth,
		Final,

		Count
	};

	struct RendererCVARs
	{
		static constexpr std::string_view ScreenSpaceAOCVarName = "r_screenSpaceAO";
		static constexpr std::string_view MinShadowResolutionVarName = "r_minShadowResolution";
		static constexpr std::string_view MaxShadowResolutionVarName = "r_maxShadowResolution";
		static constexpr std::string_view ShadowAtlasResolution = "r_shadowAtlasResolution";
		static constexpr std::string_view MaxShadowLODs = "r_maxShadowLODs";
		static constexpr std::string_view CameraOutputAttachment = "r_cameraOutputAttachment";
		static constexpr std::string_view VisualizeShadowAtlas = "r_visualizeShadowAtlas";
		static constexpr std::string_view VisualizeLightComplexity = "r_visualizeLightComplexity";

		static constexpr uint32_t MAX_SHADOW_LODS = 24;
	};

	struct RendererPipelines
	{
		/* Compute pipelines */
		static PipelineHandle m_ClusterGeneratorPipeline;
		static PipelineHandle m_ClusterCullLightPipeline;
		static PipelineHandle m_PickingPipeline;

		/* Effects pipelines */
		static PipelineHandle m_SSAOPipeline;
		static PipelineHandle m_SkyboxPipeline;

		/* Shadow rendering */
		static PipelineHandle m_ShadowRenderPipeline;
		static PipelineHandle m_TransparentShadowRenderPipeline;

		/* Debug rendering */
		static PipelineHandle m_LineRenderPipeline;

		/* SSAO */
		static PipelineHandle m_SSAOPostPassPipeline;

		/* Other */
		static PipelineHandle m_DisplayCopyPipeline;
		static PipelineHandle m_VisualizeSSAOPipeline;
		static PipelineHandle m_VisualizeObjectIDPipeline;
		static PipelineHandle m_VisualizeDepthPipeline;
		static PipelineHandle m_VisualizeLightComplexityPipeline;
	};

	struct RendererDSLayouts
	{
		/* Descriptor set layouts */
		/* Global */
		static DescriptorSetLayoutHandle m_GlobalRenderSetLayout;
		static DescriptorSetLayoutHandle m_GlobalShadowRenderSetLayout;
		static DescriptorSetLayoutHandle m_GlobalPickingSetLayout;
		static DescriptorSetLayoutHandle m_GlobalSkyboxRenderSetLayout;
		static DescriptorSetLayoutHandle m_GlobalSkyboxSamplerSetLayout;
		static DescriptorSetLayoutHandle m_GlobalLineRenderSetLayout;
		static DescriptorSetLayoutHandle m_GlobalClusterSetLayout;
		static DescriptorSetLayoutHandle m_GlobalLightSetLayout;
		static DescriptorSetLayoutHandle m_GlobalSampleDomeSetLayout;

		/* Individual */
		static DescriptorSetLayoutHandle m_CameraClusterSetLayout;
		static DescriptorSetLayoutHandle m_CameraLightSetLayout;
		static DescriptorSetLayoutHandle m_SSAOSamplersSetLayout;
		static DescriptorSetLayoutHandle m_NoiseSamplerSetLayout;
		static DescriptorSetLayoutHandle m_CameraSamplerSetLayout;
		static DescriptorSetLayoutHandle m_ShadowAtlasSamplerSetLayout;
		static DescriptorSetLayoutHandle m_ObjectDataSetLayout;
		static DescriptorSetLayoutHandle m_LightDistancesSetLayout;
		static DescriptorSetLayoutHandle m_PickingResultSetLayout;
		static DescriptorSetLayoutHandle m_PickingSamplerSetLayout;
		static DescriptorSetLayoutHandle m_DisplayCopySamplerSetLayout;
		static DescriptorSetLayoutHandle m_SSAOPostSamplerSetLayout;
		static DescriptorSetLayoutHandle m_LightGridSetLayout;
	};

	struct DummyRenderPasses
	{
		static RenderPassHandle m_DummyRenderPass;
		static RenderPassHandle m_DummySSAORenderPass;
	};

	static uint32_t* ResetLightDistances;

	constexpr size_t AttachmentNameCount = 6;
	constexpr std::string_view AttachmentNames[AttachmentNameCount] = {
		"ObjectID",
		"Color",
		"Normal",
		"AO",
		"Depth",
		"Final",
	};

	constexpr size_t DebugOverlayNameCount = 2;
	constexpr std::string_view DebugOverlayNames[AttachmentNameCount] = {
		"Shadow Atlas",
		"Light Complexity"
	};

	enum DebugOverlayBitIndices : uint8_t
	{
		ShadowAtlas = 0,
		LightComplexity = 1
	};

	constexpr size_t MaxPicks = 8;
}
