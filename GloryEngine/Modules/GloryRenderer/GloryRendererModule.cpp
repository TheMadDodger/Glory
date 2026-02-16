#include "GloryRendererModule.h"
#include "GloryRenderer.h"

#include <Engine.h>
#include <Console.h>
#include <GraphicsDevice.h>
#include <BinaryStream.h>

#include <PipelineManager.h>
#include <AssetManager.h>

#include <PipelineData.h>

GLORY_MODULE_CPP(GloryRendererModule);

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(GloryRendererModule);

	GloryRendererModule::GloryRendererModule() : m_Renderer(this)
	{
	}

	GloryRendererModule::~GloryRendererModule()
	{
	}

	void GloryRendererModule::CollectReferences(std::vector<UUID>& references)
	{
		ModuleSettings& settings = Settings();

		std::vector<UUID> newReferences;
		newReferences.push_back(settings.Value<uint64_t>("Lines Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Screen Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("SSAO Prepass Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("SSAO Blur Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Text Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Display Copy Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Skybox Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Shadows Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Shadows Transparent Textured Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Cluster Generator"));
		newReferences.push_back(settings.Value<uint64_t>("Cluster Cull Light"));
		newReferences.push_back(settings.Value<uint64_t>("Picking"));
		newReferences.push_back(settings.Value<uint64_t>("SSAO Postpass"));
		newReferences.push_back(settings.Value<uint64_t>("SSAO Visualizer"));
		newReferences.push_back(settings.Value<uint64_t>("ObjectID Visualizer"));
		newReferences.push_back(settings.Value<uint64_t>("Depth Visualizer"));
		newReferences.push_back(settings.Value<uint64_t>("Light Complexity Visualizer"));

		for (size_t i = 0; i < newReferences.size(); ++i)
		{
			if (!newReferences[i]) continue;
			references.push_back(newReferences[i]);
			Resource* pPipelineResource = m_pEngine->GetAssetManager().GetAssetImmediate(newReferences[i]);
			if (!pPipelineResource) continue;
			PipelineData* pPipelineData = static_cast<PipelineData*>(pPipelineResource);
			for (size_t i = 0; i < pPipelineData->ShaderCount(); ++i)
			{
				const UUID shaderID = pPipelineData->ShaderID(i);
				if (!shaderID) continue;
				references.push_back(shaderID);
			}
		}
	}

	void GloryRendererModule::Initialize()
	{
		m_pEngine->AddMainRenderer(&m_Renderer);
	}

	void GloryRendererModule::PostInitialize()
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();

		/* Dummy render passes */
		RenderPassInfo renderPassInfo;
		renderPassInfo.RenderTextureInfo.Width = 1;
		renderPassInfo.RenderTextureInfo.Height = 1;
		renderPassInfo.RenderTextureInfo.HasDepth = true;
		renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("object", PixelFormat::PF_RGBAI, PixelFormat::PF_R32G32B32A32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_UInt, false));
		renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R16G16B16A16Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		renderPassInfo.m_Position = RenderPassPosition::RP_Start;
		renderPassInfo.m_LoadOp = RenderPassLoadOp::OP_Clear;
		DummyRenderPasses::m_DummyRenderPass = pDevice->CreateRenderPass(std::move(renderPassInfo));

		RenderPassInfo ssaoRenderPassInfo;
		ssaoRenderPassInfo.RenderTextureInfo.Width = 1;
		ssaoRenderPassInfo.RenderTextureInfo.Height = 1;
		ssaoRenderPassInfo.RenderTextureInfo.HasDepth = false;
		ssaoRenderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("AO", PixelFormat::PF_R, PixelFormat::PF_R32Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		DummyRenderPasses::m_DummySSAORenderPass = pDevice->CreateRenderPass(std::move(ssaoRenderPassInfo));
	}

	void GloryRendererModule::Update()
	{
		ModuleSettings& settings = Settings();
		if (!settings.IsDirty()) return;

		const UUID linesPipeline = settings.Value<uint64_t>("Lines Pipeline");
		const UUID screenPipeline = settings.Value<uint64_t>("Screen Pipeline");
		const UUID SSAOPrePassPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
		const UUID SSAOBlurPipeline = settings.Value<uint64_t>("SSAO Blur Pipeline");
		const UUID textPipeline = settings.Value<uint64_t>("Text Pipeline");
		const UUID displayPipeline = settings.Value<uint64_t>("Display Copy Pipeline");
		const UUID skyboxPipeline = settings.Value<uint64_t>("Skybox Pipeline");
		const UUID irradiancePipeline = settings.Value<uint64_t>("Irradiance Pipeline");
		const UUID shadowsPipeline = settings.Value<uint64_t>("Shadows Pipeline");
		const UUID shadowsTransparentPipeline = settings.Value<uint64_t>("Shadows Transparent Textured Pipeline");
		const UUID clusterGeneratorPipeline = settings.Value<uint64_t>("Cluster Generator");
		const UUID clusterCullLightPipeline = settings.Value<uint64_t>("Cluster Cull Light");
		const UUID pickingPipeline = settings.Value<uint64_t>("Picking");
		const UUID ssaoPostpassPipeline = settings.Value<uint64_t>("SSAO Postpass");
		const UUID ssaoVisualizerPipeline = settings.Value<uint64_t>("SSAO Visualizer");
		const UUID objectIDVisualizerPipeline = settings.Value<uint64_t>("ObjectID Visualizer");
		const UUID depthVisualizerPipeline = settings.Value<uint64_t>("Depth Visualizer");
		const UUID lightComplexityVisualizerPipeline = settings.Value<uint64_t>("Light Complexity Visualizer");

		settings.SetDirty(false);
	}

	void GloryRendererModule::Draw()
	{
	}

	void GloryRendererModule::Cleanup()
	{
		for (auto& renderer : m_SecondaryRenderer)
			renderer.Cleanup();
	}

	void GloryRendererModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterAssetReference<PipelineData>("Lines Pipeline", 19);
		settings.RegisterAssetReference<PipelineData>("Screen Pipeline", 20);
		settings.RegisterAssetReference<PipelineData>("SSAO Prepass Pipeline", 21);
		settings.RegisterAssetReference<PipelineData>("SSAO Blur Pipeline", 22);
		settings.RegisterAssetReference<PipelineData>("Text Pipeline", 23);
		settings.RegisterAssetReference<PipelineData>("Display Copy Pipeline", 30);
		settings.RegisterAssetReference<PipelineData>("Skybox Pipeline", 33);
		settings.RegisterAssetReference<PipelineData>("Irradiance Pipeline", 35);
		settings.RegisterAssetReference<PipelineData>("Shadows Pipeline", 38);
		settings.RegisterAssetReference<PipelineData>("Shadows Transparent Textured Pipeline", 39);
		settings.RegisterAssetReference<PipelineData>("Cluster Generator", 44);
		settings.RegisterAssetReference<PipelineData>("Cluster Cull Light", 45);
		settings.RegisterAssetReference<PipelineData>("Picking", 47);
		settings.RegisterAssetReference<PipelineData>("SSAO Postpass", 49);
		settings.RegisterAssetReference<PipelineData>("SSAO Visualizer", 51);
		settings.RegisterAssetReference<PipelineData>("ObjectID Visualizer", 50);
		settings.RegisterAssetReference<PipelineData>("Depth Visualizer", 52);
		settings.RegisterAssetReference<PipelineData>("Light Complexity Visualizer", 54);
	}

	void GloryRendererModule::Preload()
	{
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;
		CheckCachedPipelines(pDevice);
	}

	void GloryRendererModule::OnProcessData()
	{
		if (!m_pEngine->HasData("Renderer")) return;
		m_PipelineOrder.clear();

		std::vector<char> buffer = m_pEngine->GetData("Renderer");

		BinaryMemoryStream memoryStream{ buffer };
		BinaryStream* stream = &memoryStream;

		size_t pipelineCount;
		stream->Read(pipelineCount);
		for (size_t i = 0; i < pipelineCount; ++i)
		{
			UUID pipelineID;
			stream->Read(pipelineID);
			m_PipelineOrder.emplace_back(pipelineID);
		}
	}

	void GloryRendererModule::CheckCachedPipelines(GraphicsDevice* pDevice)
	{
		const ModuleSettings& settings = Settings();
		const UUID clusterGeneratorPipeline = settings.Value<uint64_t>("Cluster Generator");
		const UUID clusterCullLightPipeline = settings.Value<uint64_t>("Cluster Cull Light");
		const UUID pickingPipeline = settings.Value<uint64_t>("Picking");
		const UUID displayPipeline = settings.Value<uint64_t>("Display Copy Pipeline");
		const UUID ssaoPostPassPipeline = settings.Value<uint64_t>("SSAO Postpass");
		const UUID visualizeSSAOPipeline = settings.Value<uint64_t>("SSAO Visualizer");
		const UUID visualizeObjectIDPipeline = settings.Value<uint64_t>("ObjectID Visualizer");
		const UUID visualizeDepthPipeline = settings.Value<uint64_t>("Depth Visualizer");
		const UUID visualizeLightComplexityPipeline = settings.Value<uint64_t>("Light Complexity Visualizer");

		static DescriptorSetLayoutHandle doubleFloatPushConstantsLayout = NULL;

		if (!doubleFloatPushConstantsLayout)
		{
			DescriptorSetLayoutInfo ssaoConstantsInfo;
			ssaoConstantsInfo.m_PushConstantRange.m_Offset = 0;
			ssaoConstantsInfo.m_PushConstantRange.m_ShaderStages = STF_Fragment;
			ssaoConstantsInfo.m_PushConstantRange.m_Size = sizeof(float) * 2;
			doubleFloatPushConstantsLayout = pDevice->CreateDescriptorSetLayout(std::move(ssaoConstantsInfo));
		}

		static DescriptorSetLayoutHandle lightComplexityPushConstantsLayout = NULL;

		if (!lightComplexityPushConstantsLayout)
		{
			DescriptorSetLayoutInfo ssaoConstantsInfo;
			ssaoConstantsInfo.m_PushConstantRange.m_Offset = 0;
			ssaoConstantsInfo.m_PushConstantRange.m_ShaderStages = STF_Fragment;
			ssaoConstantsInfo.m_PushConstantRange.m_Size = sizeof(LightComplexityConstants);
			lightComplexityPushConstantsLayout = pDevice->CreateDescriptorSetLayout(std::move(ssaoConstantsInfo));
		}

		PipelineManager& pipelines = m_pEngine->GetPipelineManager();

		/* Compute */
		PipelineData* pPipeline = pipelines.GetPipelineData(clusterGeneratorPipeline);
		RendererPipelines::m_ClusterGeneratorPipeline = pDevice->AcquireCachedComputePipeline(pPipeline, { RendererDSLayouts::m_GlobalClusterSetLayout, RendererDSLayouts::m_CameraClusterSetLayout });
		pPipeline = pipelines.GetPipelineData(clusterCullLightPipeline);
		RendererPipelines::m_ClusterCullLightPipeline = pDevice->AcquireCachedComputePipeline(pPipeline,
			{ RendererDSLayouts::m_GlobalClusterSetLayout, RendererDSLayouts::m_CameraClusterSetLayout, RendererDSLayouts::m_GlobalLightSetLayout, RendererDSLayouts::m_CameraLightSetLayout, RendererDSLayouts::m_LightDistancesSetLayout });
		pPipeline = pipelines.GetPipelineData(pickingPipeline);
		RendererPipelines::m_PickingPipeline = pDevice->AcquireCachedComputePipeline(pPipeline,
			{ RendererDSLayouts::m_GlobalPickingSetLayout, RendererDSLayouts::m_PickingResultSetLayout, RendererDSLayouts::m_PickingSamplerSetLayout });

		/* Graphics */
		pPipeline = pipelines.GetPipelineData(displayPipeline);
		RendererPipelines::m_DisplayCopyPipeline = pDevice->AcquireCachedPipeline(m_Renderer.m_FinalFrameColorPasses[0], pPipeline,
			{ RendererDSLayouts::m_DisplayCopySamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(visualizeSSAOPipeline);
		RendererPipelines::m_VisualizeSSAOPipeline = pDevice->AcquireCachedPipeline(m_Renderer.m_FinalFrameColorPasses[0], pPipeline,
			{ RendererDSLayouts::m_DisplayCopySamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(visualizeObjectIDPipeline);
		RendererPipelines::m_VisualizeObjectIDPipeline = pDevice->AcquireCachedPipeline(m_Renderer.m_FinalFrameColorPasses[0], pPipeline,
			{ RendererDSLayouts::m_DisplayCopySamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(visualizeDepthPipeline);
		RendererPipelines::m_VisualizeDepthPipeline = pDevice->AcquireCachedPipeline(m_Renderer.m_FinalFrameColorPasses[0], pPipeline,
			{ doubleFloatPushConstantsLayout, RendererDSLayouts::m_DisplayCopySamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(visualizeLightComplexityPipeline);
		RendererPipelines::m_VisualizeLightComplexityPipeline = pDevice->AcquireCachedPipeline(m_Renderer.m_FinalFrameColorPasses[0], pPipeline,
			{ lightComplexityPushConstantsLayout, RendererDSLayouts::m_DisplayCopySamplerSetLayout, RendererDSLayouts::m_LightGridSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		pPipeline = pipelines.GetPipelineData(ssaoPostPassPipeline);
		RendererPipelines::m_SSAOPostPassPipeline = pDevice->AcquireCachedPipeline(m_Renderer.m_FinalFrameColorPasses[0], pPipeline,
			{ doubleFloatPushConstantsLayout, RendererDSLayouts::m_DisplayCopySamplerSetLayout, RendererDSLayouts::m_SSAOPostSamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		const UUID shadowsPipeline = settings.Value<uint64_t>("Shadows Pipeline");
		pPipeline = pipelines.GetPipelineData(shadowsPipeline);
		RendererPipelines::m_ShadowRenderPipeline = pDevice->AcquireCachedPipeline(m_Renderer.m_ShadowsPasses[0], pPipeline,
			{ RendererDSLayouts::m_GlobalShadowRenderSetLayout, RendererDSLayouts::m_ObjectDataSetLayout }, sizeof(DefaultVertex3D),
			{ AttributeType::Float3, AttributeType::Float3, AttributeType::Float3,
			AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 });
		const UUID shadowsTransparentPipeline = settings.Value<uint64_t>("Shadows Transparent Textured Pipeline");
		pPipeline = pipelines.GetPipelineData(shadowsTransparentPipeline);
		//m_TransparentShadowRenderPipeline = pDevice->AcquireCachedPipeline(m_ShadowsPasses[0], pPipeline, {}, sizeof(DefaultVertex3D),
		//	{ AttributeType::Float3, AttributeType::Float3, AttributeType::Float3,
		//	AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 });
		const UUID ssaoPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
		pPipeline = pipelines.GetPipelineData(ssaoPipeline);
		RendererPipelines::m_SSAOPipeline = pDevice->AcquireCachedPipeline(DummyRenderPasses::m_DummySSAORenderPass, pPipeline,
			{ RendererDSLayouts::m_GlobalClusterSetLayout, RendererDSLayouts::m_GlobalSampleDomeSetLayout,
			RendererDSLayouts::m_SSAOSamplersSetLayout, RendererDSLayouts::m_NoiseSamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		const UUID skyboxPipeline = settings.Value<uint64_t>("Skybox Pipeline");
		pPipeline = pipelines.GetPipelineData(skyboxPipeline);
		RendererPipelines::m_SkyboxPipeline = pDevice->AcquireCachedPipeline(DummyRenderPasses::m_DummyRenderPass, pPipeline,
			{ RendererDSLayouts::m_GlobalSkyboxRenderSetLayout, RendererDSLayouts::m_GlobalSkyboxSamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		const UUID lineRenderPipeline = settings.Value<uint64_t>("Lines Pipeline");
		pPipeline = pipelines.GetPipelineData(lineRenderPipeline);
		RendererPipelines::m_LineRenderPipeline = pDevice->AcquireCachedPipeline(DummyRenderPasses::m_DummyRenderPass, pPipeline,
			{ RendererDSLayouts::m_GlobalLineRenderSetLayout }, sizeof(LineVertex),
			{ AttributeType::Float3, AttributeType::Float4 });
	}

	const std::vector<UUID>& GloryRendererModule::PipelineOrder() const
	{
		return m_PipelineOrder;
	}

	void GloryRendererModule::SetPipelineOrder(std::vector<UUID>&& pipelineOrder)
	{
		m_PipelineOrder = std::move(pipelineOrder);
	}

	const std::type_info& GloryRendererModule::GetModuleType()
	{
		return typeid(GloryRendererModule);
	}

	GloryRenderer* GloryRendererModule::CreateSecondaryRenderer(size_t imageCount)
	{
		GloryRenderer& renderer = m_SecondaryRenderer.emplace_back(this);
		renderer.m_GlobalSSAOSetting.m_Enabled = 0;
		renderer.m_ShadowsEnabled = false;
		renderer.m_SkyboxEnabled = false;
		renderer.m_LinesEnabled = false;
		renderer.m_ImageCount = imageCount;
		renderer.Initialize();
		return &renderer;
	}
}
