#include "EnvironmentGenerator.h"

#include "AssetPicker.h"
#include "EditorUI.h"
#include "Importer.h"
#include "EditorAssetDatabase.h"
#include "EditorPipelineManager.h"
#include "ShaderSourceData.h"

#include <AssetManager.h>
#include <CubemapData.h>
#include <GraphicsDevice.h>
#include <InternalPipeline.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Glory
{
	struct CameraRef;
	struct RenderFrame;
}

namespace Glory::Editor
{
	bool EnvironmentGenerator::m_Initialized = false;

	InternalPipeline IrradiancePipeline;
	DescriptorSetLayoutHandle ConstantsLayout;
	DescriptorSetLayoutHandle CubemapSetLayout;

	static constexpr char* EnvironmentPassName = "Environment Generator Pass";

	static constexpr uint32_t IrradianceMapResolution = 32;
	static constexpr uint32_t IrradianceMapChannels = 4;
	static constexpr uint32_t IrradianceMapBytesPerChannel = sizeof(float);
	static constexpr uint32_t IrradianceMapBytesPerPixel = IrradianceMapChannels*IrradianceMapBytesPerChannel;

	static constexpr uint32_t IrradianceMapTotalPixels = IrradianceMapResolution*IrradianceMapResolution;
	static constexpr uint32_t IrradianceMapSize = IrradianceMapTotalPixels*IrradianceMapChannels;
	static constexpr uint32_t IrradianceMapTotalByteSize = IrradianceMapSize*IrradianceMapBytesPerChannel;

	EnvironmentGenerator::EnvironmentGenerator() :
		EditorWindowTemplate("Environment Generator", 600.0f, 600.0f), m_CurrentCubemap(0),
		m_Generate(false), m_IrradianceRenderPass(NULL),
		m_pFaces {
			new float[IrradianceMapSize],
			new float[IrradianceMapSize],
			new float[IrradianceMapSize],
			new float[IrradianceMapSize],
			new float[IrradianceMapSize],
			new float[IrradianceMapSize],
		}
	{
	}

	EnvironmentGenerator::~EnvironmentGenerator()
	{
		for (size_t i = 0; i < 6; ++i)
		{
			delete[] m_pFaces[i];
		}
	}

	void EnvironmentGenerator::OnGUI()
	{
		EnvironmentPass();

		AssetManager& assets = EditorApplication::GetInstance()->GetEngine()->GetAssetManager();
		if (AssetPicker::ResourceDropdown("Cubemap", ResourceTypes::GetHash<CubemapData>(), &m_CurrentCubemap))
		{
			AssetLocation location;
			if (EditorAssetDatabase::GetAssetLocation(m_CurrentCubemap, location))
			{
				std::filesystem::path path = location.Path;
				m_Filename = path.filename().replace_extension().string();
				m_OutputPath = path.parent_path().string();
			}
		}

		Resource* pResource = m_CurrentCubemap ? assets.FindResource(m_CurrentCubemap) : nullptr;
		CubemapData* pCubemap = pResource ? static_cast<CubemapData*>(pResource) : nullptr;

		if (m_CurrentCubemap != 0 && !pCubemap)
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Cubemap not yet compiled!");
		else if (m_CurrentCubemap == 0)
			ImGui::TextUnformatted("Select a cubemap");

		ImGui::BeginDisabled(m_CurrentCubemap == 0 || !pCubemap || m_Generate);
		if (ImGui::Button("Generate Environment Maps"))
		{
			m_Generate = true;
		}
		ImGui::EndDisabled();
	}

	void EnvironmentGenerator::OnOpen()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		GraphicsDevice* pDevice = pEngine->ActiveGraphicsDevice();

		if (!m_Initialized)
			Initialize();

		TextureCreateInfo attachmentInfo;
		attachmentInfo.m_Width = attachmentInfo.m_Height = IrradianceMapResolution;
		attachmentInfo.m_ImageAspectFlags = IA_Color;
		attachmentInfo.m_ImageType = ImageType::IT_2D;
		attachmentInfo.m_PixelFormat = PixelFormat::PF_RGBA;
		attachmentInfo.m_InternalFormat = PixelFormat::PF_R32G32B32A32Sfloat;
		attachmentInfo.m_SamplingEnabled = false;
		attachmentInfo.m_Type = DataType::DT_Float;
		attachmentInfo.m_Flags = ImageFlags(IF_CopySrc);
		TextureHandle attachment = pDevice->CreateTexture(attachmentInfo);

		RenderPassInfo info;
		info.RenderTextureInfo.HasDepth = false;
		info.RenderTextureInfo.HasStencil = false;
		info.RenderTextureInfo.Width = IrradianceMapResolution;
		info.RenderTextureInfo.Height = IrradianceMapResolution;
		info.RenderTextureInfo.Attachments.push_back(Attachment("Color", attachmentInfo.m_PixelFormat,
			attachmentInfo.m_InternalFormat, attachmentInfo.m_ImageType, attachmentInfo.m_ImageAspectFlags, attachmentInfo.m_Type));
		info.RenderTextureInfo.Attachments[0].Texture = attachment;

		m_IrradianceRenderPass = pDevice->CreateRenderPass(std::move(info));

		m_IrradiancePipeline = pDevice->CreatePipeline(m_IrradianceRenderPass, &IrradiancePipeline,
			{ ConstantsLayout, CubemapSetLayout }, sizeof(glm::vec3), { AttributeType::Float3 });

		DescriptorSetInfo setInfo;
		setInfo.m_Layout = CubemapSetLayout;
		setInfo.m_Samplers.resize(1);
		setInfo.m_Samplers[0].m_TextureHandle = NULL;
		m_CubemapSet = pDevice->CreateDescriptorSet(std::move(setInfo));

		for (size_t i = 0; i < 6; ++i)
		{
			TextureCreateInfo texInfo;
			texInfo.m_Width = texInfo.m_Height = IrradianceMapResolution;
			texInfo.m_ImageAspectFlags = IA_Color;
			texInfo.m_ImageType = ImageType::IT_2D;
			texInfo.m_PixelFormat = PixelFormat::PF_RGBA;
			texInfo.m_InternalFormat = PixelFormat::PF_R32G32B32A32Sfloat;
			texInfo.m_SamplingEnabled = false;
			texInfo.m_Type = DataType::DT_Float;
			texInfo.m_Flags = ImageFlags(IF_CopyDst | IF_CopySrc);
			m_CubemapFaces[i] = pDevice->CreateTexture(texInfo);
			m_StagingBuffers[i] = pDevice->CreateBuffer(IrradianceMapTotalByteSize, BufferType::BT_TransferWrite, BufferFlags::BF_Read);
		}
	}

	void EnvironmentGenerator::OnClose()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		GraphicsDevice* pDevice = pEngine->ActiveGraphicsDevice();
		pDevice->FreePipeline(m_IrradiancePipeline);
		pDevice->FreeRenderPass(m_IrradianceRenderPass);
		pDevice->FreeDescriptorSet(m_CubemapSet);

		for (size_t i = 0; i < 6; ++i)
		{
			pDevice->FreeTexture(m_CubemapFaces[i]);
			pDevice->FreeBuffer(m_StagingBuffers[i]);
		}
	}

	void EnvironmentGenerator::EnvironmentPass()
	{
		if (!m_Generate) return;

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		GraphicsDevice* pDevice = pEngine->ActiveGraphicsDevice();
		AssetManager& assets = pEngine->GetAssetManager();

		Resource* pResource = m_CurrentCubemap ? assets.FindResource(m_CurrentCubemap) : nullptr;
		CubemapData* pCubemap = pResource ? static_cast<CubemapData*>(pResource) : nullptr;

		if (!pCubemap)
		{
			m_Generate = false;
			return;
		}

		const TextureHandle cubemapTexture = pDevice->CreateTexture(pCubemap);
		if (!cubemapTexture)
		{
			m_Generate = false;
			return;
		}

		DescriptorSetUpdateInfo setUpdateInfo;
		setUpdateInfo.m_Samplers.resize(1);
		setUpdateInfo.m_Samplers[0].m_DescriptorIndex = 0;
		setUpdateInfo.m_Samplers[0].m_TextureHandle = cubemapTexture;
		pDevice->UpdateDescriptorSet(m_CubemapSet, setUpdateInfo);

		const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

		const glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		const RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_IrradianceRenderPass);
		const TextureHandle irradianceResultTexture = pDevice->GetRenderTextureAttachment(renderTexture, 0);

		const CommandBufferHandle commandBuffer = pDevice->Begin();

		for (size_t i = 0; i < 6; ++i)
		{
			glm::mat4 constants[2] = { captureViews[i], captureProjection};

			pDevice->BeginRenderPass(commandBuffer, m_IrradianceRenderPass);
			pDevice->BeginPipeline(commandBuffer, m_IrradiancePipeline);
			pDevice->PushConstants(commandBuffer, m_IrradiancePipeline, 0, sizeof(glm::mat4)*2, constants, STF_Vertex);
			pDevice->BindDescriptorSets(commandBuffer, m_IrradiancePipeline, { m_CubemapSet });
			pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(IrradianceMapResolution), float(IrradianceMapResolution));
			pDevice->SetScissor(commandBuffer, 0, 0, IrradianceMapResolution, IrradianceMapResolution);
			/* Render irradiance */
			pDevice->DrawUnitCube(commandBuffer);
			pDevice->EndPipeline(commandBuffer);
			pDevice->EndRenderPass(commandBuffer);

			ImageBarrier irradianceBarrier;
			irradianceBarrier.m_Texture = irradianceResultTexture;
			irradianceBarrier.m_SrcAccessMask = AF_ColorAttachmentWrite;
			irradianceBarrier.m_DstAccessMask = AF_CopySrc;
			pDevice->PipelineBarrier(commandBuffer, {}, { irradianceBarrier }, PST_ColorAttachmentOutput, PST_Transfer);
			pDevice->CopyImage(commandBuffer, irradianceResultTexture, m_CubemapFaces[i]);
			pDevice->CopyImageToBuffer(commandBuffer, m_CubemapFaces[i], m_StagingBuffers[i]);
		}

		pDevice->End(commandBuffer);
		pDevice->Commit(commandBuffer);
		pDevice->Wait(commandBuffer);
		pDevice->Release(commandBuffer);

		static constexpr std::string_view sides[6] = {
			"Right",
			"Left",
			"Down",
			"Up",
			"Front",
			"Back",
		};

		/* Generate images */
		ImageData* pImages[6];
		for (size_t i = 0; i < 6; ++i)
		{
			/* Read pixels to buffer */
			pDevice->ReadBuffer(m_StagingBuffers[i], m_pFaces[i], 0, IrradianceMapTotalByteSize);

			pImages[i] = new ImageData(IrradianceMapResolution, IrradianceMapResolution,
				PixelFormat::PF_R32G32B32A32Sfloat, PixelFormat::PF_RGBA,
				IrradianceMapBytesPerPixel, std::move((char*)m_pFaces[i]), IrradianceMapTotalByteSize, false, DataType::DT_Float);
			m_pFaces[i] = new float[IrradianceMapSize];
			std::filesystem::path path = m_OutputPath;
			path.append(m_Filename + "_irradiance_" + sides[i].data()).replace_extension(".hdr");
			path = EditorAssetDatabase::GetAbsoluteAssetPath(path.string());
			Importer::Export(path, pImages[i]);
			ImportedResource importedResource{ path, pImages[i] };
			TextureData* pDefault = new TextureData(pImages[i]);
			importedResource.AddChild(pDefault, "Default");
			EditorAssetDatabase::ImportAsset(path.string(), importedResource);
		}

		/* Generate cubemap */
		CubemapData* pIrradianceMap = new CubemapData(pImages[0], pImages[1], pImages[2], pImages[3], pImages[4], pImages[5]);
		SamplerSettings& sampler = pIrradianceMap->GetSamplerSettings();
		sampler.MipmapMode = Filter::F_None;
		sampler.AddressModeU = SamplerAddressMode::SAM_ClampToEdge;
		sampler.AddressModeV = SamplerAddressMode::SAM_ClampToEdge;
		sampler.AddressModeW = SamplerAddressMode::SAM_ClampToEdge;
		std::filesystem::path path = m_OutputPath;
		path.append(m_Filename + "_irradiance").replace_extension(".gcube");
		path = EditorAssetDatabase::GetAbsoluteAssetPath(path.string());
		EditorAssetDatabase::CreateAsset(pIrradianceMap, path.string());

		m_Generate = false;
	}

	void EnvironmentGenerator::Initialize()
	{
		EditorApplication* pApplication = EditorApplication::GetInstance();
		Engine* pEngine = pApplication->GetEngine();
		GraphicsDevice* pDevice = pEngine->ActiveGraphicsDevice();

		ImportedResource vertexShader = Importer::Import("./EditorAssets/Shaders/Irradiance_Vert.shader");
		ImportedResource fragmentShader = Importer::Import("./EditorAssets/Shaders/Irradiance_Frag.shader");

		ShaderSourceData* pVertexSource = static_cast<ShaderSourceData*>(*vertexShader);
		ShaderSourceData* pFragmentSource = static_cast<ShaderSourceData*>(*fragmentShader);

		EditorPipelineManager& pipelines = pApplication->GetPipelineManager();

		std::vector<FileData*> compiledShaders(2);
		std::vector<ShaderType> shaderTypes = { ShaderType::ST_Vertex, ShaderType::ST_Fragment };

		compiledShaders[0] = pipelines.CompileShader(pVertexSource);
		compiledShaders[1] = pipelines.CompileShader(pFragmentSource);
		IrradiancePipeline.SetShaders(std::move(compiledShaders), std::move(shaderTypes));
		IrradiancePipeline.SetDepthTestEnabled(false);
		IrradiancePipeline.SetDepthWriteEnabled(false);
		IrradiancePipeline.SetBlendEnabled(false);
		IrradiancePipeline.GetCullFace() = CullFace::None;

		DescriptorSetLayoutInfo layoutInfo;
		layoutInfo.m_PushConstantRange.m_Offset = 0;
		layoutInfo.m_PushConstantRange.m_ShaderStages = STF_Vertex;
		layoutInfo.m_PushConstantRange.m_Size = sizeof(glm::mat4)*2;
		ConstantsLayout = pDevice->CreateDescriptorSetLayout(std::move(layoutInfo));

		layoutInfo = DescriptorSetLayoutInfo();
		layoutInfo.m_Samplers.resize(1);
		layoutInfo.m_SamplerNames = { "cubemap" };
		layoutInfo.m_Samplers[0].m_BindingIndex = 0;
		layoutInfo.m_Samplers[0].m_ShaderStages = STF_Fragment;
		CubemapSetLayout = pDevice->CreateDescriptorSetLayout(std::move(layoutInfo));

		m_Initialized = true;
	}
}
