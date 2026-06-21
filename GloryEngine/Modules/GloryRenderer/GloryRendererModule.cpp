#include "GloryRendererModule.h"
#include "GloryRenderer.h"

#include <IEngine.h>
#include <Console.h>
#include <GraphicsDevice.h>
#include <BinaryStream.h>

#include <PipelineManager.h>
#include <Resources.h>

#include <PipelineData.h>

GLORY_MODULE_CPP(GloryRendererModule);

#define CHECK_REQUIRED_PIPELINE(pipeline, name) if (!pipeline)\
debug.LogError("Missing " name ", engine will be unable to render any objects!");

#define CHECK_PIPELINE(pipeline, name) if (!pipeline)\
debug.LogWarning("Missing " name ", some features of the renderer may not work!");

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(GloryRendererModule);

	GloryRendererModule::GloryRendererModule() : m_Renderer(this)
	{
		for (size_t i = 0; i < MaxSecondaryRenderers; ++i)
			m_SecondaryRenderers[i].SetModule(this);
	}

	GloryRendererModule::~GloryRendererModule()
	{
	}

	void GloryRendererModule::CollectReferences(std::vector<UUID>& references)
	{
		std::vector<UUID> newReferences;
		newReferences.push_back(m_Settings->m_LinesPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_SSAOPrepassPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_SSAOBlurPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_TextPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_DisplayCopyPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_SkyboxPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_ShadowsPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_ShadowsTransparentTexturePipeline.GetUUID());
		newReferences.push_back(m_Settings->m_ClusterPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_ClusterCullLightPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_PickingPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_SSAOPostpassPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_SSAOVisPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_ObjectIDVisPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_DepthVisPipeline.GetUUID());
		newReferences.push_back(m_Settings->m_LightComplexityVisPipeline.GetUUID());

		for (size_t i = 0; i < newReferences.size(); ++i)
		{
			if (!newReferences[i]) continue;
			references.push_back(newReferences[i]);
			Resource* pPipelineResource = m_pEngine->GetResources().GetResource(newReferences[i]);
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

	void GloryRendererModule::RegisterTypes()
	{
		Reflect::SetReflectInstance(&m_pEngine->Reflection());
		Reflect::RegisterType<GloryRendererSettings>();
	}

	void GloryRendererModule::InitializeSettings()
	{
		SetSettings(&m_Settings);
		m_Settings.InsertGroupBefore(SETTING_NAME(GloryRendererSettings::m_LinesPipeline), "Pipelines");

		/* Default values for asset references need to be set during load otherwise we don't have a resource manager yet */
		m_Settings->m_LinesPipeline = UUID(19ull);
		m_Settings->m_SSAOPrepassPipeline = UUID(21ull);
		m_Settings->m_SSAOBlurPipeline = UUID(4ull);
		m_Settings->m_TextPipeline = UUID(23ull);
		m_Settings->m_DisplayCopyPipeline = UUID(30ull);
		m_Settings->m_SkyboxPipeline = UUID(33ull);
		m_Settings->m_ShadowsPipeline = UUID(38ull);
		m_Settings->m_ShadowsTransparentTexturePipeline = UUID(39ull);
		m_Settings->m_ClusterPipeline = UUID(44ull);
		m_Settings->m_ClusterCullLightPipeline = UUID(45ull);
		m_Settings->m_PickingPipeline = UUID(47ull);
		m_Settings->m_SSAOPostpassPipeline = UUID(49ull);
		m_Settings->m_SSAOVisPipeline = UUID(51ull);
		m_Settings->m_ObjectIDVisPipeline = UUID(50ull);
		m_Settings->m_DepthVisPipeline = UUID(52ull);
		m_Settings->m_LightComplexityVisPipeline = UUID(54ull);
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
		renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_UByte));
		renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R16G16B16A16Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Short));
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
		settings.SetDirty(false);
	}

	void GloryRendererModule::Draw()
	{
	}

	void GloryRendererModule::Cleanup()
	{
		for (size_t i = 0; i < m_NextSecundaryRenderer; ++i)
			m_SecondaryRenderers[i].Cleanup();
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

		Utils::BinaryMemoryStream memoryStream{ buffer };
		Utils::BinaryStream* stream = &memoryStream;

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
		const UUID clusterGeneratorPipeline = m_Settings->m_ClusterPipeline.GetUUID();
		const UUID clusterCullLightPipeline = m_Settings->m_ClusterCullLightPipeline.GetUUID();
		const UUID pickingPipeline = m_Settings->m_PickingPipeline.GetUUID();
		const UUID displayPipeline = m_Settings->m_DisplayCopyPipeline.GetUUID();
		const UUID ssaoPostPassPipeline = m_Settings->m_SSAOPostpassPipeline.GetUUID();
		const UUID visualizeSSAOPipeline = m_Settings->m_SSAOVisPipeline.GetUUID();
		const UUID visualizeObjectIDPipeline = m_Settings->m_ObjectIDVisPipeline.GetUUID();
		const UUID visualizeDepthPipeline = m_Settings->m_DepthVisPipeline.GetUUID();
		const UUID visualizeLightComplexityPipeline = m_Settings->m_LightComplexityVisPipeline.GetUUID();

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
		const UUID shadowsPipeline = m_Settings->m_ShadowsPipeline.GetUUID();
		pPipeline = pipelines.GetPipelineData(shadowsPipeline);
		RendererPipelines::m_ShadowRenderPipeline = pDevice->AcquireCachedPipeline(m_Renderer.m_ShadowsPasses[0], pPipeline,
			{ RendererDSLayouts::m_GlobalShadowRenderSetLayout, RendererDSLayouts::m_ObjectDataSetLayout }, sizeof(DefaultVertex3D),
			{ AttributeType::Float3, AttributeType::Float3, AttributeType::Float3,
			AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 });
		const UUID shadowsTransparentPipeline = m_Settings->m_ShadowsTransparentTexturePipeline.GetUUID();
		pPipeline = pipelines.GetPipelineData(shadowsTransparentPipeline);
		//m_TransparentShadowRenderPipeline = pDevice->AcquireCachedPipeline(m_ShadowsPasses[0], pPipeline, {}, sizeof(DefaultVertex3D),
		//	{ AttributeType::Float3, AttributeType::Float3, AttributeType::Float3,
		//	AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 });
		const UUID ssaoPipeline = m_Settings->m_SSAOPrepassPipeline.GetUUID();
		pPipeline = pipelines.GetPipelineData(ssaoPipeline);
		RendererPipelines::m_SSAOPipeline = pDevice->AcquireCachedPipeline(DummyRenderPasses::m_DummySSAORenderPass, pPipeline,
			{ RendererDSLayouts::m_GlobalClusterSetLayout, RendererDSLayouts::m_GlobalSampleDomeSetLayout,
			RendererDSLayouts::m_SSAOSamplersSetLayout, RendererDSLayouts::m_NoiseSamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		const UUID ssaoBlurPipeline = m_Settings->m_SSAOBlurPipeline.GetUUID();
		pPipeline = pipelines.GetPipelineData(ssaoBlurPipeline);
		RendererPipelines::m_SSAOBlurPipeline = pDevice->AcquireCachedPipeline(DummyRenderPasses::m_DummySSAORenderPass, pPipeline,
			{ RendererDSLayouts::m_GlobalBlurSetLayout, RendererDSLayouts::m_DisplayCopySamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		const UUID skyboxPipeline = m_Settings->m_SkyboxPipeline.GetUUID();
		pPipeline = pipelines.GetPipelineData(skyboxPipeline);
		RendererPipelines::m_SkyboxPipeline = pDevice->AcquireCachedPipeline(DummyRenderPasses::m_DummyRenderPass, pPipeline,
			{ RendererDSLayouts::m_GlobalSkyboxRenderSetLayout, RendererDSLayouts::m_GlobalSkyboxSamplerSetLayout },
			sizeof(glm::vec3), { AttributeType::Float3 });
		const UUID lineRenderPipeline = m_Settings->m_LinesPipeline.GetUUID();
		pPipeline = pipelines.GetPipelineData(lineRenderPipeline);
		RendererPipelines::m_LineRenderPipeline = pDevice->AcquireCachedPipeline(DummyRenderPasses::m_DummyRenderPass, pPipeline,
			{ RendererDSLayouts::m_GlobalLineRenderSetLayout }, sizeof(LineVertex),
			{ AttributeType::Float3, AttributeType::Float4 });

		if (m_FirstPipelineCacheCheck)
		{
			Debug& debug = m_pEngine->GetDebug();
			CHECK_REQUIRED_PIPELINE(RendererPipelines::m_ClusterGeneratorPipeline, "Cluster Generator");
			CHECK_REQUIRED_PIPELINE(RendererPipelines::m_ClusterCullLightPipeline, "Cluster Cull Light");
			CHECK_REQUIRED_PIPELINE(RendererPipelines::m_DisplayCopyPipeline, "Display Copy Pipeline");

			CHECK_PIPELINE(RendererPipelines::m_PickingPipeline, "Picking");
			CHECK_PIPELINE(RendererPipelines::m_SSAOPipeline, "SSAO Prepass Pipeline");
			CHECK_PIPELINE(RendererPipelines::m_SSAOBlurPipeline, "SSAO Blur Pipeline");
			CHECK_PIPELINE(RendererPipelines::m_SSAOPostPassPipeline, "SSAO Postpass");
			CHECK_PIPELINE(RendererPipelines::m_SkyboxPipeline, "Skybox Pipeline");
			CHECK_PIPELINE(RendererPipelines::m_ShadowRenderPipeline, "Shadows Pipeline");
			CHECK_PIPELINE(RendererPipelines::m_TransparentShadowRenderPipeline, "Shadows Transparent Textured Pipeline");
			CHECK_PIPELINE(RendererPipelines::m_LineRenderPipeline, "Lines Pipeline");

			CHECK_PIPELINE(RendererPipelines::m_VisualizeSSAOPipeline, "SSAO Visualizer");
			CHECK_PIPELINE(RendererPipelines::m_VisualizeObjectIDPipeline, "ObjectID Visualizer");
			CHECK_PIPELINE(RendererPipelines::m_VisualizeDepthPipeline, "Depth Visualizer");
			CHECK_PIPELINE(RendererPipelines::m_VisualizeLightComplexityPipeline, "Light Complexity Visualizer");

			m_FirstPipelineCacheCheck = false;
		}
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
		GLORY_ASSERT(m_NextSecundaryRenderer < MaxSecondaryRenderers, "Max number of secundary renderers reached!");

		GloryRenderer& renderer = m_SecondaryRenderers[m_NextSecundaryRenderer];
		++m_NextSecundaryRenderer;
		renderer.SetSSAOEnabled(false);
		renderer.SetShadowsEnabled(false);
		renderer.SetSkyboxEnabled(false);
		renderer.SetLinesEnabled(false);
		renderer.SetIsMainRenderer(false);
		renderer.m_ImageCount = imageCount;
		renderer.Initialize();
		return &renderer;
	}
}
