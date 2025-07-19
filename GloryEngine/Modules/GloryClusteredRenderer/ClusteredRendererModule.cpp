#include "ClusteredRendererModule.h"

#include <AssetManager.h>
#include <MaterialManager.h>
#include <Engine.h>
#include <GraphicsModule.h>
#include <GPUResourceManager.h>
#include <FileLoaderModule.h>
#include <CameraManager.h>
#include <InternalMaterial.h>
#include <InternalPipeline.h>
#include <SceneManager.h>
#include <GScene.h>
#include <FontData.h>
#include <CubemapData.h>
#include <FontDataStructs.h>
#include <Console.h>
#include <GPUTextureAtlas.h>

#include <DistributedRandom.h>

#include <glm/gtx/matrix_decompose.hpp>

namespace Glory
{
	static constexpr std::string_view ScreenSpaceAOCVarName = "r_screenSpaceAO";
	static constexpr std::string_view MinShadowResolutionVarName = "r_minShadowResolution";
	static constexpr std::string_view MaxShadowResolutionVarName = "r_maxShadowResolution";
	static constexpr std::string_view ShadowAtlasResolution = "r_shadowAtlasResolution";
	static constexpr std::string_view MaxShadowLODs = "r_maxShadowLODs";

	static uint32_t* ResetLightDistances;

	GLORY_MODULE_VERSION_CPP(ClusteredRendererModule);

	ClusteredRendererModule::ClusteredRendererModule() :
		m_MinShadowResolution(256), m_MaxShadowResolution(2048), m_ShadowAtlasResolution(8192),
		m_ShadowMapResolutions{}, m_pShadowAtlas(nullptr), m_MaxShadowLODs(6)
	{
	}

	ClusteredRendererModule::~ClusteredRendererModule()
	{
	}

	void ClusteredRendererModule::GetCameraRenderTextureInfos(std::vector<RenderTextureCreateInfo>& infos)
	{
		infos.resize(2);
		RenderTextureCreateInfo& mainTetxure = infos[0];
		mainTetxure.HasDepth = true;
		mainTetxure.Attachments.push_back(Attachment("object", PixelFormat::PF_RGBAI, PixelFormat::PF_R32G32B32A32Uint, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_UInt, false));
		mainTetxure.Attachments.push_back(Attachment("Debug", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		mainTetxure.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		mainTetxure.Attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R16G16B16A16Sfloat, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		mainTetxure.Attachments.push_back(Attachment("AOBlurred", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		mainTetxure.Attachments.push_back(Attachment("Data", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));

		RenderTextureCreateInfo& effectsTexture = infos[1];
		effectsTexture.Attachments.push_back(Attachment("AO", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
	}

	void ClusteredRendererModule::OnCameraResize(CameraRef camera)
	{
		// When the camera rendertexture resizes we need to generate a new grid of clusters for that camera
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		Buffer* pClusterSSBO = nullptr;
		if (!camera.GetUserData<Buffer>("ClusterSSBO", pClusterSSBO)) return; // Should not happen but just in case
		GenerateClusterSSBO(pClusterSSBO, camera);
	}

	void ClusteredRendererModule::OnCameraPerspectiveChanged(CameraRef camera)
	{
		// When the camera changed perspective we need to generate a new grid of clusters for that camera
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		Buffer* pClusterSSBO = nullptr;
		if (!camera.GetUserData<Buffer>("ClusterSSBO", pClusterSSBO)) return; // Should not happen but just in case
		GenerateClusterSSBO(pClusterSSBO, camera);
	}

	MaterialData* ClusteredRendererModule::GetInternalMaterial(std::string_view name) const
	{
		if (name == "irradiance")
			return m_pIrradianceMaterialData;
		return nullptr;
	}

	void ClusteredRendererModule::CollectReferences(std::vector<UUID>& references)
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

	void ClusteredRendererModule::Initialize()
	{
		RendererModule::Initialize();
		m_pEngine->GetConsole().RegisterCVar({ std::string{ ScreenSpaceAOCVarName }, "Enables/disables screen space ambient occlusion.", float(m_GlobalSSAOSetting.m_Enabled), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ MinShadowResolutionVarName }, "Sets the minimum resolution for shadow maps.", float(m_MinShadowResolution), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ MaxShadowResolutionVarName }, "Sets the maximum resolution for shadow maps.", float(m_MaxShadowResolution), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ ShadowAtlasResolution }, "Sets the resolution for the shadow atlas.", float(m_ShadowAtlasResolution), CVar::Flags::Save });
		m_pEngine->GetConsole().RegisterCVar({ std::string{ MaxShadowLODs }, "Sets the number of shadow map LODs.", float(m_MaxShadowLODs), CVar::Flags::Save });

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ ScreenSpaceAOCVarName }, [this](const CVar* cvar) {
			m_GlobalSSAOSetting.m_Enabled = cvar->m_Value == 1.0f;
			m_GlobalSSAOSetting.m_Dirty = true;
		});

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ MinShadowResolutionVarName }, [this](const CVar* cvar) {
			ResizeShadowMapLODResolutions(uint32_t(cvar->m_Value), m_MaxShadowResolution);
		});

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ MaxShadowResolutionVarName }, [this](const CVar* cvar) {
			ResizeShadowMapLODResolutions(m_MinShadowResolution, uint32_t(cvar->m_Value));
		});

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ ShadowAtlasResolution }, [this](const CVar* cvar) {
			ResizeShadowAtlas(uint32_t(cvar->m_Value));
		});

		m_pEngine->GetConsole().RegisterCVarChangeHandler(std::string{ MaxShadowLODs }, [this](const CVar* cvar) {
			GenerateShadowLODDivisions(cvar->m_Value);
			ResizeShadowMapLODResolutions(m_MinShadowResolution, m_MaxShadowResolution);
		});

		AddRenderPass(RP_Prepass, RenderPass{ "Prepare Data Pass", [this](CameraRef, const RenderFrame& frame) {
			/* Update light data */
			const uint32_t count = (uint32_t)std::fmin(m_FrameData.ActiveLights.count(), MAX_LIGHTS);
			m_pLightCountSSBO->Assign(&count, 0, sizeof(uint32_t));
			m_pLightsSSBO->Assign(m_FrameData.ActiveLights.data(), 0, MAX_LIGHTS*sizeof(LightData));
			m_pLightSpaceTransformsSSBO->Assign(m_FrameData.LightSpaceTransforms.data(), 0, MAX_LIGHTS*sizeof(glm::mat4));
			m_pLightDistancesSSBO->Assign(ResetLightDistances);
		} });

		AddRenderPass(RP_PreCompositePass, RenderPass{ "Shadows Pass", [this](CameraRef camera, const RenderFrame& frame) {
			ShadowMapsPass(camera, frame);
			m_pLightsSSBO->Assign(m_FrameData.ActiveLights.data(), 0, MAX_LIGHTS*sizeof(LightData));
		} });
		
		AddRenderPass(RP_ObjectPass, RenderPass{ "Indirect Object Pass", [this](CameraRef camera, const RenderFrame& frame) {
			GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
			GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

			for (PipelineRenderData& pipelineRenderData : m_StaticPipelineRenderDatas)
			{
				if (pipelineRenderData.m_pCombinedMesh->VertexCount() == 0 || pipelineRenderData.m_IndirectDrawCommands->empty()) continue;

				if (!pipelineRenderData.m_pIndirectDrawCommandsBuffer)
				{
					pipelineRenderData.m_pIndirectDrawCommandsBuffer = pResourceManager->CreateBuffer(1, BufferBindingTarget::B_DRAW_INDIRECT, MemoryUsage::MU_STATIC_DRAW, 0);
					pipelineRenderData.m_IndirectDrawCommands.m_Dirty = true;
				}

				if (!pipelineRenderData.m_pIndirectDrawPerObjectDataBuffer)
				{
					pipelineRenderData.m_pIndirectDrawPerObjectDataBuffer = pResourceManager->CreateBuffer(1, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_DRAW, 4);
					pipelineRenderData.m_PerObjectData.m_Dirty = true;
				}

				if (pipelineRenderData.m_IndirectDrawCommands)
					pipelineRenderData.m_pIndirectDrawCommandsBuffer->Assign(pipelineRenderData.m_IndirectDrawCommands->data(),
						pipelineRenderData.m_IndirectDrawCommands->size()*sizeof(DrawElementsIndirectCommand));
				if (pipelineRenderData.m_PerObjectData)
					pipelineRenderData.m_pIndirectDrawPerObjectDataBuffer->Assign(pipelineRenderData.m_PerObjectData->data(),
						pipelineRenderData.m_PerObjectData->size()*sizeof(PerObjectData));

				Mesh* pMesh = pResourceManager->CreateMesh(pipelineRenderData.m_pCombinedMesh);

				MaterialData* pMaterialData = m_pEngine->GetMaterialManager().GetMaterial(pipelineRenderData.m_Materials->at(0));
				if (!pMaterialData) continue;
				Material* pMaterial = pGraphics->UseMaterial(pMaterialData);
				if (!pMaterial) continue;

				ObjectData object;
				object.Model = glm::identity<glm::mat4>();
				object.View = camera.GetView();
				object.Projection = camera.GetProjection();
				object.ObjectID = 0;
				object.SceneID = 0;

				pMaterial->SetProperties(m_pEngine);
				pMaterial->SetObjectData(object);
				pGraphics->EnableDepthWrite(true);

				pipelineRenderData.m_pIndirectDrawCommandsBuffer->BindForDraw();
				pipelineRenderData.m_pIndirectDrawPerObjectDataBuffer->BindForDraw();
				pGraphics->MultiDrawMeshIndirect(pMesh, pipelineRenderData.m_IndirectDrawCommands->size());
				pGraphics->EnableDepthWrite(true);
				pipelineRenderData.m_pIndirectDrawCommandsBuffer->Unbind();
				pipelineRenderData.m_pIndirectDrawPerObjectDataBuffer->Unbind();
			}

		} });
	}

	void ClusteredRendererModule::OnPostInitialize()
	{
		FileImportSettings importSettings;
		importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
		importSettings.AddNullTerminateAtEnd = true;

		// Cluster generator shader
		std::filesystem::path path;
		GetResourcePath("Shaders/Compute/ClusterShader.shader", path);
		m_pClusterShaderData = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(path.string(), importSettings);
		m_pClusterShaderPipelineData = new InternalPipeline({ m_pClusterShaderData }, { ShaderType::ST_Compute });
		m_pClusterShaderMaterialData = new InternalMaterial(m_pClusterShaderPipelineData);

		// Active cluster marker shader
		GetResourcePath("Shaders/Compute/MarkActiveClusters.shader", path);
		m_pMarkActiveClustersShaderData = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(path.string(), importSettings);
		m_pMarkActiveClustersPipelineData = new InternalPipeline({ m_pMarkActiveClustersShaderData }, { ShaderType::ST_Compute });
		m_pMarkActiveClustersMaterialData = new InternalMaterial(m_pMarkActiveClustersPipelineData);

		// Compact active clusters shader
		GetResourcePath("Shaders/Compute/BuildCompactClusterList.shader", path);
		m_pCompactClustersShaderData = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(path.string(), importSettings);
		m_pCompactClustersPipelineData = new InternalPipeline({ m_pCompactClustersShaderData }, { ShaderType::ST_Compute });
		m_pCompactClustersMaterialData = new InternalMaterial(m_pCompactClustersPipelineData);

		// Light culling shader
		GetResourcePath("Shaders/Compute/ClusterCullLight.shader", path);
		m_pClusterCullLightShaderData = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load(path.string(), importSettings);
		m_pClusterCullLightPipelineData = new InternalPipeline({ m_pClusterCullLightShaderData }, { ShaderType::ST_Compute });
		m_pClusterCullLightMaterialData = new InternalMaterial(m_pClusterCullLightPipelineData);

		const ModuleSettings& settings = Settings();
		const UUID screenPipeline = settings.Value<uint64_t>("Screen Pipeline");
		const UUID SSAOPrePassPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
		const UUID SSAOBlurPipeline = settings.Value<uint64_t>("SSAO Blur Pipeline");
		const UUID textPipeline = settings.Value<uint64_t>("Text Pipeline");
		const UUID displayPipeline = settings.Value<uint64_t>("Display Copy Pipeline");
		const UUID skyboxPipeline = settings.Value<uint64_t>("Skybox Pipeline");
		const UUID irradiancePipeline = settings.Value<uint64_t>("Irradiance Pipeline");
		const UUID shadowsPipeline = settings.Value<uint64_t>("Shadows Pipeline");
		const UUID shadowsTransparentPipeline = settings.Value<uint64_t>("Shadows Transparent Textured Pipeline");

		m_pDeferredCompositeMaterial = new MaterialData();
		m_pDeferredCompositeMaterial->SetPipeline(screenPipeline);
		m_pDisplayCopyMaterial = new MaterialData();
		m_pDisplayCopyMaterial->SetPipeline(displayPipeline);
		m_pSSRMaterial = new MaterialData();
		m_pSSAOMaterial = new MaterialData();
		m_pSSAOMaterial->SetPipeline(SSAOPrePassPipeline);
		m_pSSAOBlurMaterial = new MaterialData();
		m_pSSAOBlurMaterial->SetPipeline(SSAOBlurPipeline);
		m_pTextMaterialData = new MaterialData();
		m_pTextMaterialData->SetPipeline(textPipeline);
		m_pSkyboxMaterialData = new MaterialData();
		m_pSkyboxMaterialData->SetPipeline(skyboxPipeline);
		m_pIrradianceMaterialData = new MaterialData();
		m_pIrradianceMaterialData->SetPipeline(irradiancePipeline);
		m_pShadowsMaterialData = new MaterialData();
		m_pShadowsMaterialData->SetPipeline(shadowsPipeline);
		m_pShadowsTransparentMaterialData = new MaterialData();
		m_pShadowsTransparentMaterialData->SetPipeline(shadowsTransparentPipeline);

		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		m_pScreenToViewSSBO = pResourceManager->CreateBuffer(sizeof(ScreenToView), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_DRAW, 2);
		m_pScreenToViewSSBO->Assign(NULL);

		m_pLightsSSBO = pResourceManager->CreateBuffer(sizeof(LightData)*MAX_LIGHTS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_DRAW, 3);
		m_pLightsSSBO->Assign(NULL);
		m_pLightCountSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_DRAW, 7);
		m_pLightCountSSBO->Assign(NULL);
		m_pLightSpaceTransformsSSBO = pResourceManager->CreateBuffer(sizeof(glm::mat4)*MAX_LIGHTS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_DRAW, 8);
		m_pLightSpaceTransformsSSBO->Assign(NULL);
		m_pLightDistancesSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t)*MAX_LIGHTS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_DRAW, 6);

		ResetLightDistances = new uint32_t[MAX_LIGHTS];
		for (size_t i = 0; i < MAX_LIGHTS; ++i)
			ResetLightDistances[i] = NUM_DEPTH_SLICES;
		m_pLightDistancesSSBO->Assign(ResetLightDistances);

		m_pSSAOSettingsSSBO = pResourceManager->CreateBuffer(sizeof(SSAOSettings), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_DRAW, 6);
		m_pSSAOSettingsSSBO->Assign(NULL);

		GenerateDomeSamplePointsSSBO(pResourceManager, 64);

		m_pClusterShaderMaterial = pResourceManager->CreateMaterial(m_pClusterShaderMaterialData);
		m_pMarkActiveClustersMaterial = pResourceManager->CreateMaterial(m_pMarkActiveClustersMaterialData);
		m_pCompactClustersMaterial = pResourceManager->CreateMaterial(m_pCompactClustersMaterialData);
		m_pClusterCullLightMaterial = pResourceManager->CreateMaterial(m_pClusterCullLightMaterialData);

		const uint32_t vertexBufferSize = 4*sizeof(VertexPosColorTex);
		const uint32_t indexBufferSize = 6*sizeof(uint32_t);
		m_pQuadMeshVertexBuffer = pResourceManager->CreateBuffer(vertexBufferSize, BufferBindingTarget::B_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);
		m_pQuadMeshIndexBuffer = pResourceManager->CreateBuffer(indexBufferSize, BufferBindingTarget::B_ELEMENT_ARRAY, MemoryUsage::MU_STATIC_DRAW, 0);

		const uint32_t indices[6] = {
			0, 1, 2,
			2, 3, 0
		};
		const VertexPosColorTex defaultVertices[4] = {
			{{-1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
			{{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
			{{1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
			{{1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		};

		m_pQuadMeshIndexBuffer->Assign(indices);
		m_pQuadMeshVertexBuffer->Assign(defaultVertices);
		m_pQuadMesh = pResourceManager->CreateMesh(4, 6, InputRate::Vertex, 0, sizeof(VertexPosColorTex),
			PrimitiveType::PT_Triangles, { AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }, m_pQuadMeshVertexBuffer, m_pQuadMeshIndexBuffer);

		m_pShadowAtlas = CreateGPUTextureAtlas({ m_ShadowAtlasResolution, m_ShadowAtlasResolution }, true);

		GenerateShadowLODDivisions(m_MaxShadowLODs);
		GenerateShadowMapLODResolutions();
	}

	void ClusteredRendererModule::Update()
	{
		ModuleSettings& settings = Settings();
		if (!settings.IsDirty()) return;

		const UUID screenPipeline = settings.Value<uint64_t>("Screen Pipeline");
		const UUID SSAOPrePassPipeline = settings.Value<uint64_t>("SSAO Prepass Pipeline");
		const UUID SSAOBlurPipeline = settings.Value<uint64_t>("SSAO Blur Pipeline");
		const UUID textPipeline = settings.Value<uint64_t>("Text Pipeline");
		const UUID displayPipeline = settings.Value<uint64_t>("Display Copy Pipeline");
		const UUID skyboxPipeline = settings.Value<uint64_t>("Skybox Pipeline");
		const UUID irradiancePipeline = settings.Value<uint64_t>("Irradiance Pipeline");
		const UUID shadowsPipeline = settings.Value<uint64_t>("Shadows Pipeline");
		const UUID shadowsTransparentPipeline = settings.Value<uint64_t>("Shadows Transparent Textured Pipeline");

		m_pDeferredCompositeMaterial->SetPipeline(screenPipeline);
		m_pDisplayCopyMaterial->SetPipeline(displayPipeline);
		m_pSSAOMaterial->SetPipeline(SSAOPrePassPipeline);
		m_pSSAOBlurMaterial->SetPipeline(SSAOBlurPipeline);
		m_pTextMaterialData->SetPipeline(textPipeline);
		m_pSkyboxMaterialData->SetPipeline(skyboxPipeline);
		m_pIrradianceMaterialData->SetPipeline(irradiancePipeline);
		m_pShadowsMaterialData->SetPipeline(shadowsPipeline);
		m_pShadowsTransparentMaterialData->SetPipeline(shadowsTransparentPipeline);

		settings.SetDirty(false);
	}

	void ClusteredRendererModule::Cleanup()
	{
		delete m_pClusterShaderData;
		m_pClusterShaderData = nullptr;

		delete m_pMarkActiveClustersShaderData;
		m_pMarkActiveClustersShaderData = nullptr;

		delete m_pCompactClustersShaderData;
		m_pCompactClustersShaderData = nullptr;

		delete m_pClusterCullLightShaderData;
		m_pClusterCullLightShaderData = nullptr;

		delete m_pClusterShaderMaterialData;
		m_pClusterShaderMaterialData = nullptr;

		delete m_pMarkActiveClustersMaterialData;
		m_pMarkActiveClustersMaterialData = nullptr;

		delete m_pCompactClustersMaterialData;
		m_pCompactClustersMaterialData = nullptr;

		delete m_pClusterCullLightMaterialData;
		m_pClusterCullLightMaterialData = nullptr;

		delete m_pDeferredCompositeMaterial;
		m_pDeferredCompositeMaterial = nullptr;
		
		delete m_pDisplayCopyMaterial;
		m_pDisplayCopyMaterial = nullptr;

		delete m_pSSRMaterial;
		m_pSSRMaterial = nullptr;

		delete m_pSSAOMaterial;
		m_pSSAOMaterial = nullptr;

		delete m_pSSAOBlurMaterial;
		m_pSSAOBlurMaterial = nullptr;

		delete m_pTextMaterialData;
		m_pTextMaterialData = nullptr;
		
		delete m_pSkyboxMaterialData;
		m_pSkyboxMaterialData = nullptr;
		
		delete m_pIrradianceMaterialData;
		m_pIrradianceMaterialData = nullptr;
		
		delete m_pShadowsMaterialData;
		m_pShadowsMaterialData = nullptr;

		delete m_pShadowAtlas;
		m_pShadowAtlas = nullptr;
	}

	void ClusteredRendererModule::OnRender(CameraRef camera, const RenderData& renderData, const std::vector<LightData>&)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		Resource* pMeshResource = m_pEngine->GetAssetManager().FindResource(renderData.m_MeshID);
		if (!pMeshResource) return;
		MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);
		MaterialData* pMaterialData = m_pEngine->GetMaterialManager().GetMaterial(renderData.m_MaterialID);
		if (!pMaterialData) return;
		Material* pMaterial = pGraphics->UseMaterial(pMaterialData);
		if (!pMaterial) return;

		ObjectData object;
		object.Model = renderData.m_World;
		object.View = camera.GetView();
		object.Projection = camera.GetProjection();
		object.ObjectID = renderData.m_ObjectID;
		object.SceneID = renderData.m_SceneID;

		pMaterial->SetProperties(m_pEngine);
		pMaterial->SetObjectData(object);
		pGraphics->EnableDepthWrite(renderData.m_DepthWrite);
		pGraphics->DrawMesh(pMeshData, 0, pMeshData->VertexCount());
		pGraphics->EnableDepthWrite(true);
	}

	void ClusteredRendererModule::OnRender(CameraRef camera, const TextRenderData& renderData, const std::vector<LightData>& lights)
	{
		if (renderData.m_Text.empty()) return;

		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		Resource* pFontResource = m_pEngine->GetAssetManager().FindResource(renderData.m_FontID);
		if (!pFontResource) return;
		FontData* pFontData = static_cast<FontData*>(pFontResource);
		if (!pFontData) return;
		if (!m_pTextMaterialData) return;

		if (!m_pTextMaterial)
		{
			m_pTextMaterial = pResourceManager->CreateMaterial(m_pTextMaterialData);
			if (!m_pTextMaterial) return;
		}
		m_pTextMaterial->Use();

		ObjectData object;
		object.Model = renderData.m_World;
		object.View = camera.GetView();
		object.Projection = camera.GetProjection();
		object.ObjectID = renderData.m_ObjectID;
		object.SceneID = renderData.m_SceneID;

		m_pTextMaterial->SetProperties(m_pEngine);
		m_pTextMaterial->SetObjectData(object);

		InternalTexture* pTextureData = pFontData->GetGlyphTexture();
		if (!pTextureData) return;

		Texture* pTexture = pResourceManager->CreateTexture((TextureData*)pTextureData);
		if (pTexture) m_pTextMaterial->SetTexture("texSampler", pTexture);

		auto iter = m_pTextMeshes.find(renderData.m_ObjectID);
		const bool exists = iter != m_pTextMeshes.end();
		if (!exists)
		{
			MeshData* pMesh = new MeshData(renderData.m_Text.size()*4, sizeof(VertexPosColorTex),
				{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
			iter = m_pTextMeshes.emplace(renderData.m_ObjectID, pMesh).first;
		}

		const float textWrap = renderData.m_TextWrap*renderData.m_Scale*pFontData->FontHeight();
		if (renderData.m_TextDirty || !exists)
			Utils::GenerateTextMesh(iter->second.get(), pFontData, renderData, textWrap);

		Mesh* pMesh = pResourceManager->CreateMesh(iter->second.get());
		pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
	}

	void ClusteredRendererModule::OnRenderEffects(CameraRef camera, RenderTexture* pRenderTexture)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		/* Render SSAO */
		Material* pMaterial = pGraphics->UseMaterial(m_pSSAOMaterial);

		static SSAOSettings DefaultSSAO{};

		GScene* pActiveScene = m_pEngine->GetSceneManager()->GetActiveScene();
		SSAOSettings& ssao = pActiveScene ? pActiveScene->Settings().m_SSAOSettings : DefaultSSAO;

		if (pActiveScene && (ssao.m_Dirty || m_GlobalSSAOSetting.m_Dirty))
		{
			const bool hasAO = ssao.m_Enabled;
			ssao.m_Enabled &= m_GlobalSSAOSetting.m_Enabled;
			m_pSSAOSettingsSSBO->Assign(&ssao, sizeof(SSAOSettings));
			ssao.m_Dirty = false;
			m_GlobalSSAOSetting.m_Dirty = false;
			ssao.m_Enabled = hasAO;
		}
		if (!ssao.m_Enabled || !m_GlobalSSAOSetting.m_Enabled) return;

		GenerateDomeSamplePointsSSBO(pGraphics->GetResourceManager(), ssao.m_KernelSize);

		pGraphics->EnableDepthTest(false);

		camera.GetRenderTexture(1)->BindForDraw();
		pRenderTexture->BindAll(pMaterial);
		pMaterial->SetTexture("Noise", m_pSampleNoiseTexture);

		m_pSamplePointsDomeSSBO->BindForDraw();
		m_pScreenToViewSSBO->BindForDraw();
		m_pSSAOSettingsSSBO->BindForDraw();

		/* Draw the screen quad */
		pGraphics->DrawScreenQuad();

		m_pSamplePointsDomeSSBO->Unbind();
		m_pScreenToViewSSBO->Unbind();
		m_pSSAOSettingsSSBO->Unbind();

		// Reset render textures and materials
		camera.GetRenderTexture(1)->UnBindForDraw();
		pGraphics->UseMaterial(nullptr);

		/* Blur SSAO */
		pMaterial = pGraphics->UseMaterial(m_pSSAOBlurMaterial);

		pRenderTexture->BindForDraw();
		camera.GetRenderTexture(1)->BindAll(pMaterial);

		m_pSSAOSettingsSSBO->BindForDraw();

		// Draw the triangles !
		pGraphics->DrawScreenQuad();

		m_pSSAOSettingsSSBO->Unbind();

		// Reset render textures and materials
		pRenderTexture->UnBindForDraw();
		pGraphics->UseMaterial(nullptr);
		pGraphics->EnableDepthTest(true);
	}

	void ClusteredRendererModule::OnDoCompositing(CameraRef camera, uint32_t width, uint32_t height, RenderTexture* pRenderTexture)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		GScene* pActiveScene = m_pEngine->GetSceneManager()->GetActiveScene();
		const UUID irradianceMapID = pActiveScene ? pActiveScene->Settings().m_LightingSettings.m_IrradianceMap : 0;
		Resource* pIrradianceResource = irradianceMapID ? m_pEngine->GetAssetManager().FindResource(irradianceMapID) : nullptr;
		CubemapData* pIrradianceMap = pIrradianceResource ? static_cast<CubemapData*>(pIrradianceResource) : nullptr;
		Texture* pIrradianceTexture = pIrradianceMap ? pGraphics->GetResourceManager()->CreateCubemapTexture(pIrradianceMap) : 0;

		Buffer* pClusterSSBO = nullptr;
		Buffer* pLightIndexSSBO = nullptr;
		Buffer* pLightGridSSBO = nullptr;
		if (!camera.GetUserData("ClusterSSBO", pClusterSSBO)) return;
		if (!camera.GetUserData("LightIndexSSBO", pLightIndexSSBO)) return;
		if (!camera.GetUserData("LightGridSSBO", pLightGridSSBO)) return;

		pGraphics->EnableDepthTest(false);
		pGraphics->SetViewport(0, 0, width, height);

		glm::uvec2 resolution = camera.GetResolution();
		glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);
		float zNear = camera.GetNear();
		float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x / (float)gridSize.x), (uint32_t)std::ceilf(resolution.y / (float)gridSize.y));
		ScreenToView screenToView;
		screenToView.ProjectionInverse = camera.GetProjectionInverse();
		screenToView.ViewInverse = camera.GetViewInverse();
		screenToView.ScreenDimensions = resolution;
		screenToView.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		screenToView.Scale = (float)gridSize.z / std::log2f(zFar / zNear);
		screenToView.Bias = -((float)gridSize.z * std::log2f(zNear) / std::log2f(zFar / zNear));
		screenToView.zNear = zNear;
		screenToView.zFar = zFar;

		m_pScreenToViewSSBO->Assign((void*)&screenToView);

		/* Render final image */
		Material* pMaterial = pGraphics->UseMaterial(m_pDeferredCompositeMaterial);

		pRenderTexture->BindAll(pMaterial);
		pMaterial->SetCubemapTexture("IrradianceMap", pIrradianceTexture ? pIrradianceTexture : nullptr);
		pMaterial->SetTexture("ShadowAtlas", m_pShadowAtlas->GetTexture());
		pMaterial->SetPropertiesBuffer(m_pEngine, 7);

		pClusterSSBO->BindForDraw();
		m_pScreenToViewSSBO->BindForDraw();
		m_pLightsSSBO->BindForDraw();
		pLightIndexSSBO->BindForDraw();
		pLightGridSSBO->BindForDraw();
		m_pLightSpaceTransformsSSBO->BindForDraw();

		/* Draw the screen quad */
		pGraphics->DrawScreenQuad();

		pClusterSSBO->Unbind();
		m_pScreenToViewSSBO->Unbind();
		m_pLightsSSBO->Unbind();
		pLightIndexSSBO->Unbind();
		pLightGridSSBO->Unbind();
		m_pLightSpaceTransformsSSBO->Unbind();

		// Reset render textures and materials
		pGraphics->UseMaterial(nullptr);
		pGraphics->EnableDepthTest(true);
	}

	void ClusteredRendererModule::OnDisplayCopy(RenderTexture* pRenderTexture, uint32_t width, uint32_t height)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		pGraphics->EnableDepthTest(false);
		pGraphics->SetViewport(0, 0, width, height);

		Material* pMaterial = pGraphics->UseMaterial(m_pDisplayCopyMaterial);

		pRenderTexture->BindAll(pMaterial);

		/* Draw the screen quad */
		pGraphics->DrawScreenQuad();

		/* Reset render textures and materials */
		pGraphics->UseMaterial(nullptr);
		pGraphics->EnableDepthTest(true);
	}

	void ClusteredRendererModule::OnRenderSkybox(CameraRef camera, CubemapData* pCubemap)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		Texture* pCubemapTexture = pGraphics->GetResourceManager()->CreateCubemapTexture(pCubemap);
		if (!pCubemapTexture) return;
		Material* pMaterial = pGraphics->UseMaterial(m_pSkyboxMaterialData);
		if (!pMaterial) return;

		ObjectData object;
		object.Model = glm::identity<glm::mat4>();
		object.View = glm::mat4(glm::mat3(camera.GetView()));
		object.Projection = camera.GetProjection();
		object.SceneID = 0;
		object.ObjectID = 0;
		pMaterial->SetObjectData(object);

		/* Set skybox texture */
		pMaterial->SetCubemapTexture("skybox", pCubemapTexture);

		/* Draw the skybox */
		pGraphics->DrawUnitCube();

		/* Reset render textures and materials */
		pGraphics->UseMaterial(nullptr);
	}

	void ClusteredRendererModule::OnStartCameraRender(CameraRef camera, const FrameData<LightData>& lights)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		Buffer* pClusterSSBO = nullptr;
		if (!camera.GetUserData<Buffer>("ClusterSSBO", pClusterSSBO))
		{
			Buffer* pLightIndexSSBO = nullptr;
			Buffer* pLightGridSSBO = nullptr;

			pClusterSSBO = pResourceManager->CreateBuffer(sizeof(VolumeTileAABB)*NUM_CLUSTERS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 1);
			pClusterSSBO->Assign(NULL);

			pLightIndexSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t)*(NUM_CLUSTERS*MAX_LIGHTS_PER_TILE + 1), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_COPY, 4);
			pLightIndexSSBO->Assign(NULL);

			pLightGridSSBO = pResourceManager->CreateBuffer(sizeof(LightGrid)*NUM_CLUSTERS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_COPY, 5);
			pLightGridSSBO->Assign(NULL);

			camera.SetUserData("ClusterSSBO", pClusterSSBO);
			camera.SetUserData("LightIndexSSBO", pLightIndexSSBO);
			camera.SetUserData("LightGridSSBO", pLightGridSSBO);

			GenerateClusterSSBO(pClusterSSBO, camera);
		}
	}

	void ClusteredRendererModule::OnEndCameraRender(CameraRef camera, const FrameData<LightData>& lights)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		RenderTexture* pRenderTexture = m_pEngine->GetCameraManager().GetRenderTextureForCamera(camera, m_pEngine);
		Texture* pDepthTexture = pRenderTexture->GetTextureAttachment("Depth");

		glm::uvec2 resolution = camera.GetResolution();
		glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

		Buffer* pClusterSSBO = nullptr;
		Buffer* pLightIndexSSBO = nullptr;
		Buffer* pLightGridSSBO = nullptr;
		if (!camera.GetUserData("ClusterSSBO", pClusterSSBO)) return;
		if (!camera.GetUserData("LightIndexSSBO", pLightIndexSSBO)) return;
		if (!camera.GetUserData("LightGridSSBO", pLightGridSSBO)) return;

		float zNear = camera.GetNear();
		float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x / (float)gridSize.x), (uint32_t)std::ceilf(resolution.y / (float)gridSize.y));
		ScreenToView screenToView;
		screenToView.ProjectionInverse = camera.GetProjectionInverse();
		screenToView.ViewInverse = camera.GetViewInverse();
		screenToView.ScreenDimensions = resolution;
		screenToView.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		screenToView.Scale = (float)gridSize.z / std::log2f(zFar / zNear);
		screenToView.Bias = -((float)gridSize.z * std::log2f(zNear) / std::log2f(zFar / zNear));
		screenToView.zNear = zNear;
		screenToView.zFar = zFar;

		m_pScreenToViewSSBO->Assign((void*)&screenToView);

		m_pClusterCullLightMaterial->Use();
		m_pClusterCullLightMaterial->SetMatrix4("viewMatrix", camera.GetView());
		pClusterSSBO->BindForDraw();
		m_pScreenToViewSSBO->BindForDraw();
		m_pLightsSSBO->BindForDraw();
		pLightIndexSSBO->BindForDraw();
		pLightGridSSBO->BindForDraw();
		m_pLightDistancesSSBO->BindForDraw();
		m_pLightCountSSBO->BindForDraw();
		pGraphics->DispatchCompute(1, 1, 6);
		pClusterSSBO->Unbind();
		m_pScreenToViewSSBO->Unbind();
		m_pLightsSSBO->Unbind();
		pLightIndexSSBO->Unbind();
		pLightGridSSBO->Unbind();
		m_pLightDistancesSSBO->Unbind();
		m_pLightCountSSBO->Unbind();
	}

	void ClusteredRendererModule::LoadSettings(ModuleSettings& settings)
	{
		RendererModule::LoadSettings(settings);
		settings.RegisterAssetReference<PipelineData>("Screen Pipeline", 20);
		settings.RegisterAssetReference<PipelineData>("SSAO Prepass Pipeline", 21);
		settings.RegisterAssetReference<PipelineData>("SSAO Blur Pipeline", 22);
		settings.RegisterAssetReference<PipelineData>("Text Pipeline", 23);
		settings.RegisterAssetReference<PipelineData>("Display Copy Pipeline", 30);
		settings.RegisterAssetReference<PipelineData>("Skybox Pipeline", 33);
		settings.RegisterAssetReference<PipelineData>("Irradiance Pipeline", 35);
		settings.RegisterAssetReference<PipelineData>("Shadows Pipeline", 38);
		settings.RegisterAssetReference<PipelineData>("Shadows Transparent Textured Pipeline", 39);
	}

	size_t ClusteredRendererModule::GetGCD(size_t a, size_t b)
	{
		if (b == 0)
			return a;
		return GetGCD(b, a % b);
	}

	void ClusteredRendererModule::GenerateClusterSSBO(Buffer* pBuffer, CameraRef camera)
	{
		const glm::uvec2 resolution = camera.GetResolution();
		const glm::uvec3 gridSize = glm::vec3(m_GridSizeX, m_GridSizeY, NUM_DEPTH_SLICES);

		const float zNear = camera.GetNear();
		const float zFar = camera.GetFar();

		const uint32_t sizeX = std::max((uint32_t)std::ceilf(resolution.x / (float)gridSize.x), (uint32_t)std::ceilf(resolution.y / (float)gridSize.y));
		ScreenToView screenToView;
		screenToView.ProjectionInverse = camera.GetProjectionInverse();
		screenToView.ViewInverse = camera.GetViewInverse();
		screenToView.ScreenDimensions = resolution;
		screenToView.TileSizes = glm::uvec4(gridSize.x, gridSize.y, gridSize.z, sizeX);
		screenToView.Scale = (float)gridSize.z / std::log2f(zFar / zNear);
		screenToView.Bias = -((float)gridSize.z * std::log2f(zNear) / std::log2f(zFar / zNear));
		screenToView.zNear = zNear;
		screenToView.zFar = zFar;

		m_pScreenToViewSSBO->Assign((void*)&screenToView);

		m_pClusterShaderMaterial->Use();
		pBuffer->BindForDraw();
		m_pScreenToViewSSBO->BindForDraw();
		m_pEngine->GetMainModule<GraphicsModule>()->DispatchCompute(gridSize.x, gridSize.y, gridSize.z);
		pBuffer->Unbind();
		m_pScreenToViewSSBO->Unbind();
	}

	float lerp(float a, float b, float f)
	{
		return a + f*(b - a);
	}

	void ClusteredRendererModule::GenerateDomeSamplePointsSSBO(GPUResourceManager* pResourceManager, uint32_t size)
	{
		if (m_SSAOKernelSize == size) return;
		m_SSAOKernelSize = size;

		if (!m_pSamplePointsDomeSSBO)
		{
			m_pSamplePointsDomeSSBO = pResourceManager->CreateBuffer(sizeof(glm::vec3)*MAX_KERNEL_SIZE, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_DRAW, 3);
			m_pSamplePointsDomeSSBO->Assign(NULL);
		}

		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		std::vector<glm::vec3> samplePoints{ m_SSAOKernelSize, glm::vec3{} };
		for (unsigned int i = 0; i < m_SSAOKernelSize; ++i)
		{
			samplePoints[i] = glm::vec3{
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			};
			samplePoints[i] = glm::normalize(samplePoints[i]);
			samplePoints[i] *= randomFloats(generator);

			float scale = float(i)/m_SSAOKernelSize;
			scale = lerp(0.1f, 1.0f, scale*scale);
			samplePoints[i] *= scale;
		}

		m_pSamplePointsDomeSSBO->BindForDraw();
		m_pSamplePointsDomeSSBO->Assign(samplePoints.data(), 0, sizeof(glm::vec3) * m_SSAOKernelSize);
		m_pSamplePointsDomeSSBO->Unbind();

		const size_t textureSize = 4;

		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < textureSize*textureSize; ++i)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		TextureCreateInfo textureInfo;
		textureInfo.m_Width = textureSize;
		textureInfo.m_Height = textureSize;
		textureInfo.m_PixelFormat = PixelFormat::PF_RGB;
		textureInfo.m_InternalFormat = PixelFormat::PF_R16G16B16A16Sfloat;
		textureInfo.m_ImageType = ImageType::IT_2D;
		textureInfo.m_Type = DataType::DT_Float;
		m_pSampleNoiseTexture = m_pEngine->GetMainModule<GraphicsModule>()->GetResourceManager()->CreateTexture(std::move(textureInfo), static_cast<const void*>(ssaoNoise.data()));
	}

	void ClusteredRendererModule::ShadowMapsPass(CameraRef camera, const RenderFrame& frameData)
	{
		if (m_FrameData.ActiveLights.count() == 0) return;

		uint32_t lightDepthSlices[MAX_LIGHTS];
		m_pLightDistancesSSBO->Read(&lightDepthSlices, 0, m_FrameData.ActiveLights.count()*sizeof(uint32_t));
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		const uint32_t sliceSteps = NUM_DEPTH_SLICES/m_MaxShadowLODs;

		m_pShadowAtlas->ReleaseAllChunks();
		m_pShadowAtlas->Bind();
		for (size_t i = 0; i < m_FrameData.ActiveLights.count(); ++i)
		{
			auto& lightData = m_FrameData.ActiveLights[i];
			const auto& lightTransform = m_FrameData.LightSpaceTransforms[i];
			const auto& lightID = m_FrameData.ActiveLightIDs[i];

			if (!lightData.shadowsEnabled) continue;

			const uint32_t depthSlice = lightDepthSlices[i];
			/* No need to render that which can't be seen! */
			if (depthSlice == NUM_DEPTH_SLICES)
			{
				lightData.shadowsEnabled = 0;
				continue;
			}

			const uint32_t shadowLOD = std::min(depthSlice/sliceSteps, uint32_t(m_MaxShadowLODs - 1));
			const glm::uvec2 shadowMapResolution = m_ShadowMapResolutions[shadowLOD];

			const UUID chunkID = m_pShadowAtlas->ReserveChunk(shadowMapResolution.x, shadowMapResolution.y, lightID);
			if (!chunkID)
			{
				lightData.shadowsEnabled = 0;
				m_pEngine->GetDebug().LogError("Failed to reserve chunk in shadow atlas, there is not enough space left.");
				continue;
			}

			pGraphics->SetCullFace(CullFace::Front);
			pGraphics->SetColorMask(false, false, false, false);
			pGraphics->EnableDepthWrite(true);
			pGraphics->EnableDepthTest(true);
			m_pShadowAtlas->BindChunk(chunkID);
			for (size_t j = 0; j < m_FrameData.ObjectsToRender.size(); ++j)
			{
				const auto& objectToRender = m_FrameData.ObjectsToRender[j];
				RenderShadow(i, m_FrameData, objectToRender);
			}
			pGraphics->SetColorMask(true, true, true, true);
			pGraphics->SetCullFace(CullFace::None);
			lightData.shadowCoords = m_pShadowAtlas->GetChunkCoords(lightID);
		}
		m_pShadowAtlas->Unbind();
	}

	void ClusteredRendererModule::RenderShadow(size_t lightIndex, const RenderFrame& frameData, const RenderData& objectToRender)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		Resource* pMeshResource = m_pEngine->GetAssetManager().FindResource(objectToRender.m_MeshID);
		if (!pMeshResource) return;
		MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);
		MaterialData* pMaterialData = m_pEngine->GetMaterialManager().GetMaterial(objectToRender.m_MaterialID);
		if (!pMaterialData) return;
		/* @todo Check if attached pipeline has the TRANSPARENT feature enabled */
		Material* pMaterial = pGraphics->UseMaterial(m_pShadowsMaterialData);
		if (!pMaterial) return;

		ObjectData object;
		object.Model = objectToRender.m_World;
		object.View = frameData.LightSpaceTransforms[lightIndex];
		object.Projection = glm::identity<glm::mat4>();
		object.ObjectID = objectToRender.m_ObjectID;
		object.SceneID = objectToRender.m_SceneID;
		pMaterial->SetProperties(m_pEngine);
		pMaterial->SetObjectData(object);
		pGraphics->DrawMesh(pMeshData, 0, pMeshData->VertexCount());

		pGraphics->UseMaterial(nullptr);
	}

	void ClusteredRendererModule::GenerateShadowLODDivisions(uint32_t maxLODs)
	{
		m_MaxShadowLODs = std::min(MAX_SHADOW_LODS, maxLODs);
		m_ShadowLODDivisions.clear();
		m_ShadowLODDivisions.reserve(m_MaxShadowLODs);
		for (size_t i = 0; i < m_MaxShadowLODs; ++i)
		{
			const uint32_t divider = uint32_t(pow(2, i));
			m_ShadowLODDivisions.push_back(divider);
		}
	}

	void ClusteredRendererModule::GenerateShadowMapLODResolutions()
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		m_ShadowMapResolutions.reserve(m_MaxShadowLODs);
		for (size_t i = 0; i < m_MaxShadowLODs; ++i)
		{
			const uint32_t res = std::max(m_MinShadowResolution, m_MaxShadowResolution/m_ShadowLODDivisions[i]);
			m_ShadowMapResolutions.push_back({ res, res });
		}
	}

	void ClusteredRendererModule::ResizeShadowMapLODResolutions(uint32_t minSize, uint32_t maxSize)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		m_MinShadowResolution = minSize;
		m_MaxShadowResolution = maxSize;
		m_ShadowMapResolutions.reserve(m_MaxShadowLODs);

		for (size_t i = 0; i < m_MaxShadowLODs; ++i)
		{
			const uint32_t res = std::max(m_MinShadowResolution, m_MaxShadowResolution/m_ShadowLODDivisions[i]);
			if (i >= m_ShadowMapResolutions.size())
			{
				m_ShadowMapResolutions.push_back({ res, res });
				continue;
			}
			m_ShadowMapResolutions[i] = { res, res };
		}
	}

	void ClusteredRendererModule::ResizeShadowAtlas(uint32_t newSize)
	{
		m_ShadowAtlasResolution = newSize;
		if (!m_pShadowAtlas) return;
		m_pShadowAtlas->Resize(m_ShadowAtlasResolution);
	}
}
