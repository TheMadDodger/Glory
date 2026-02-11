#include "GloryRendererData.h"

namespace Glory
{
	/* Compute pipelines */
	PipelineHandle RendererPipelines::m_ClusterGeneratorPipeline = NULL;
	PipelineHandle RendererPipelines::m_ClusterCullLightPipeline = NULL;
	PipelineHandle RendererPipelines::m_PickingPipeline = NULL;

	/* Effects pipelines */
	PipelineHandle RendererPipelines::m_SSAOPipeline = NULL;
	PipelineHandle RendererPipelines::m_SkyboxPipeline = NULL;

	/* Shadow rendering */
	PipelineHandle RendererPipelines::m_ShadowRenderPipeline = NULL;
	PipelineHandle RendererPipelines::m_TransparentShadowRenderPipeline = NULL;

	/* Debug rendering */
	PipelineHandle RendererPipelines::m_LineRenderPipeline = NULL;

	/* SSAO */
	PipelineHandle RendererPipelines::m_SSAOPostPassPipeline = NULL;

	/* Other */
	PipelineHandle RendererPipelines::m_DisplayCopyPipeline = NULL;
	PipelineHandle RendererPipelines::m_VisualizeSSAOPipeline = NULL;
	PipelineHandle RendererPipelines::m_VisualizeObjectIDPipeline = NULL;
	PipelineHandle RendererPipelines::m_VisualizeDepthPipeline = NULL;
	PipelineHandle RendererPipelines::m_VisualizeLightComplexityPipeline = NULL;

	/* Descriptor set layouts */
	/* Global */
	DescriptorSetLayoutHandle RendererDSLayouts::m_GlobalRenderSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_GlobalShadowRenderSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_GlobalPickingSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_GlobalSkyboxRenderSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_GlobalSkyboxSamplerSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_GlobalLineRenderSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_GlobalClusterSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_GlobalLightSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_GlobalSampleDomeSetLayout = NULL;

	/* Individual */
	DescriptorSetLayoutHandle RendererDSLayouts::m_CameraClusterSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_CameraLightSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_SSAOSamplersSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_NoiseSamplerSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_CameraSamplerSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_ShadowAtlasSamplerSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_ObjectDataSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_LightDistancesSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_PickingResultSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_PickingSamplerSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_DisplayCopySamplerSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_SSAOPostSamplerSetLayout = NULL;
	DescriptorSetLayoutHandle RendererDSLayouts::m_LightGridSetLayout = NULL;

	RenderPassHandle DummyRenderPasses::m_DummyRenderPass = NULL;
	RenderPassHandle DummyRenderPasses::m_DummySSAORenderPass = NULL;
}
