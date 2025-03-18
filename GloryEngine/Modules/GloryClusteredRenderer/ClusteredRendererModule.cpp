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
#include <FontDataStructs.h>

#include <DistributedRandom.h>

#include <glm/gtx/matrix_decompose.hpp>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(ClusteredRendererModule);

	ClusteredRendererModule::ClusteredRendererModule()
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
		mainTetxure.Attachments.push_back(Attachment("Normal", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
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

	void ClusteredRendererModule::CollectReferences(std::vector<UUID>& references)
	{
		ModuleSettings& settings = Settings();

		const size_t start = references.size();
		references.push_back(settings.Value<uint64_t>("Lines Pipeline"));
		references.push_back(settings.Value<uint64_t>("Screen Pipeline"));
		references.push_back(settings.Value<uint64_t>("SSAO Prepass Pipeline"));
		references.push_back(settings.Value<uint64_t>("SSAO Blur Pipeline"));
		references.push_back(settings.Value<uint64_t>("Text Pipeline"));
		references.push_back(settings.Value<uint64_t>("Display Copy Pipeline"));
		const size_t end = references.size();

		for (size_t i = start; i < end; ++i)
		{
			Resource* pPipelineResource = m_pEngine->GetAssetManager().GetAssetImmediate(references[i]);
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

		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		m_pScreenToViewSSBO = pResourceManager->CreateBuffer(sizeof(ScreenToView), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_DYNAMIC_DRAW, 2);
		m_pScreenToViewSSBO->Assign(NULL);

		m_pLightsSSBO = pResourceManager->CreateBuffer(sizeof(LightData) * MAX_LIGHTS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_DRAW, 3);
		m_pLightsSSBO->Assign(NULL);

		m_pSSAOSettingsSSBO = pResourceManager->CreateBuffer(sizeof(SSAOSettings), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_DRAW, 6);
		m_pSSAOSettingsSSBO->Assign(NULL);

		GenerateDomeSamplePointsSSBO(pResourceManager, 64);

		m_pClusterShaderMaterial = pResourceManager->CreateMaterial(m_pClusterShaderMaterialData);
		m_pMarkActiveClustersMaterial = pResourceManager->CreateMaterial(m_pMarkActiveClustersMaterialData);
		m_pCompactClustersMaterial = pResourceManager->CreateMaterial(m_pCompactClustersMaterialData);
		m_pClusterCullLightMaterial = pResourceManager->CreateMaterial(m_pClusterCullLightMaterialData);

		uint32_t vertexBufferSize = 4 * sizeof(VertexPosColorTex);
		uint32_t indexBufferSize = 6 * sizeof(uint32_t);
		m_pQuadMeshVertexBuffer = pResourceManager->CreateBuffer(vertexBufferSize, BufferBindingTarget::B_ARRAY, MemoryUsage::MU_DYNAMIC_DRAW, 0);
		m_pQuadMeshIndexBuffer = pResourceManager->CreateBuffer(indexBufferSize, BufferBindingTarget::B_ELEMENT_ARRAY, MemoryUsage::MU_DYNAMIC_DRAW, 0);

		uint32_t indices[6] = {
			0, 1, 2,
			2, 3, 0
		};
		VertexPosColorTex defaultVertices[4] = {
			{{-1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
			{{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
			{{1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
			{{1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		};

		m_pQuadMeshIndexBuffer->Assign(indices, 6 * sizeof(uint32_t));
		m_pQuadMeshVertexBuffer->Assign(defaultVertices, 4 * sizeof(VertexPosColorTex));
		m_pQuadMesh = pResourceManager->CreateMesh(4, 6, InputRate::Vertex, 0, sizeof(VertexPosColorTex),
			PrimitiveType::PT_Triangles, { AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }, m_pQuadMeshVertexBuffer, m_pQuadMeshIndexBuffer);
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

	void GenerateTextMesh(MeshData* pMesh, FontData* pFontData, const TextRenderData& renderData)
	{
		const std::string_view text = renderData.m_Text;
		const glm::vec4& color = renderData.m_Color;
		const float scale = renderData.m_Scale;
		const Alignment alignment = renderData.m_Alignment;
		const float textWrap = renderData.m_TextWrap*scale*pFontData->FontHeight();

		pMesh->ClearVertices();
		pMesh->ClearIndices();

		/* Get words */
		std::vector<std::pair<size_t, size_t>> wordPositions;
		size_t currentStringPos = 0;

		while (currentStringPos < text.size())
		{
			const size_t nextSpace = text.find(' ', currentStringPos);
			if (nextSpace == std::string::npos)
			{
				wordPositions.push_back({ currentStringPos, text.size() - currentStringPos });
				break;
			}
			wordPositions.push_back({ currentStringPos, nextSpace - currentStringPos });
			currentStringPos = nextSpace + 1;
		}

		float writeX = 0.0f;
		float writeY = 0.0f;
		size_t letterCount = 0;
		/* For generating the mesh one line at a time */
		std::function<void(const std::string_view, float)> drawLine = [&](const std::string_view line, float lineWidth) {
			switch (alignment)
			{
			case Alignment::Left:
				writeX = 0.0f;
				break;
			case Alignment::Center:
				writeX = -lineWidth / 2.0f;
				break;
			case Alignment::Right:
				writeX = -lineWidth;
				break;
			default:
				break;
			}

			for (char c : line)
			{
				const size_t glyphIndex = pFontData->GetGlyphIndex(c);
				const GlyphData* glyph = pFontData->GetGlyph(glyphIndex);

				if (!glyph) continue;

				const float xpos = writeX + glyph->Bearing.x * scale;
				const float ypos = writeY - (glyph->Size.y - glyph->Bearing.y) * scale;

				const float w = glyph->Size.x * scale;
				const float h = glyph->Size.y * scale;

				VertexPosColorTex vertices[4] = {
					{ { xpos, ypos + h, }, color, { glyph->Coords.x, glyph->Coords.y } },
					{ { xpos, ypos, }, color, { glyph->Coords.x, glyph->Coords.w } },
					{ { xpos + w, ypos, }, color, { glyph->Coords.z, glyph->Coords.w } },
					{ { xpos + w, ypos + h, }, color, { glyph->Coords.z, glyph->Coords.y }, }
				};

				pMesh->AddVertex(reinterpret_cast<float*>(&vertices[0]));
				pMesh->AddVertex(reinterpret_cast<float*>(&vertices[1]));
				pMesh->AddVertex(reinterpret_cast<float*>(&vertices[2]));
				pMesh->AddVertex(reinterpret_cast<float*>(&vertices[3]));
				pMesh->AddFace(letterCount * 4 + 0, letterCount * 4 + 1, letterCount * 4 + 2, letterCount * 4 + 3);
				++letterCount;
				writeX += (glyph->Advance >> 6) * scale;
			}

			writeY -= pFontData->FontHeight() * scale;
		};

		/* Calculate lines and generate the mesh */
		/* Process words into lines */
		size_t currentLineLength = 0;
		float currentLineWidth = 0.0f;
		size_t lineStart = 0;

		const size_t spaceGlyphIndex = pFontData->GetGlyphIndex(' ');
		const GlyphData* spaceGlyph = pFontData->GetGlyph(spaceGlyphIndex);

		const float space = spaceGlyph ? (spaceGlyph->Advance >> 6) * scale : 0.0f;
		for (size_t i = 0; i < wordPositions.size(); ++i)
		{
			const std::pair<size_t, size_t>& wordPosition = wordPositions[i];
			const std::string_view word = text.substr(wordPosition.first, wordPosition.second);

			/* Calculate word width */
			float wordWidth = 0.0f;
			for (char c : word)
			{
				const size_t glyphIndex = pFontData->GetGlyphIndex(c);
				const GlyphData* glyph = pFontData->GetGlyph(glyphIndex);

				if (!glyph) continue;
				const float advance = (glyph->Advance >> 6)*scale;
				wordWidth += advance;
			}

			if (currentLineLength > 0 && textWrap > 0.0f && currentLineWidth + wordWidth >= textWrap)
			{
				const std::string_view line = text.substr(lineStart, wordPosition.first - lineStart);
				drawLine(line, currentLineWidth);
				lineStart = wordPosition.first;
				currentLineWidth = 0.0f;
				currentLineLength = 0;
				--i;
				continue;
			}

			currentLineWidth += wordWidth + space;
			currentLineLength += word.length() + 1;
		}

		/* If there was 1 more line we could not generate yet we can generate it now */
		if (currentLineLength > 0)
		{
			const std::string_view line = text.substr(lineStart);
			drawLine(line, currentLineWidth);
		}
	}

	void ClusteredRendererModule::OnRender(CameraRef camera, const TextRenderData& renderData, const std::vector<LightData>& lights)
	{
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

		if (renderData.m_TextDirty || !exists)
			GenerateTextMesh(iter->second.get(), pFontData, renderData);

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

		if (pActiveScene && ssao.m_Dirty)
		{
			m_pSSAOSettingsSSBO->Assign(&ssao, sizeof(SSAOSettings));
			ssao.m_Dirty = false;
		}
		if (!ssao.m_Enabled) return;

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

	void ClusteredRendererModule::OnDoCompositing(CameraRef camera, const FrameData<LightData>& lights, uint32_t width, uint32_t height, RenderTexture* pRenderTexture)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		Buffer* pClusterSSBO = nullptr;
		Buffer* pLightIndexSSBO = nullptr;
		Buffer* pLightGridSSBO = nullptr;
		if (!camera.GetUserData("ClusterSSBO", pClusterSSBO)) return;
		if (!camera.GetUserData("LightIndexSSBO", pLightIndexSSBO)) return;
		if (!camera.GetUserData("LightGridSSBO", pLightGridSSBO)) return;

		pGraphics->EnableDepthTest(false);
		pGraphics->SetViewport(0, 0, width, height);

		uint32_t count = (uint32_t)std::fmin(lights.size(), MAX_LIGHTS);
		m_pLightsSSBO->Assign(lights.data(), 0, count * sizeof(LightData));

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

		pClusterSSBO->BindForDraw();
		m_pScreenToViewSSBO->BindForDraw();
		m_pLightsSSBO->BindForDraw();
		pLightIndexSSBO->BindForDraw();
		pLightGridSSBO->BindForDraw();

		/* Draw the screen quad */
		pGraphics->DrawScreenQuad();

		pClusterSSBO->Unbind();
		m_pScreenToViewSSBO->Unbind();
		m_pLightsSSBO->Unbind();
		pLightIndexSSBO->Unbind();
		pLightGridSSBO->Unbind();

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

	void ClusteredRendererModule::OnStartCameraRender(CameraRef camera, const FrameData<LightData>& lights)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		Buffer* pClusterSSBO = nullptr;
		if (!camera.GetUserData<Buffer>("ClusterSSBO", pClusterSSBO))
		{
			Buffer* pActiveClustersSSBO = nullptr;
			Buffer* pActiveUniqueClustersSSBO = nullptr;
			Buffer* pLightIndexSSBO = nullptr;
			Buffer* pLightGridSSBO = nullptr;

			pClusterSSBO = pResourceManager->CreateBuffer(sizeof(VolumeTileAABB) * NUM_CLUSTERS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 1);
			pClusterSSBO->Assign(NULL);

			pActiveClustersSSBO = pResourceManager->CreateBuffer(sizeof(bool) * NUM_CLUSTERS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 1);
			pActiveClustersSSBO->Assign(NULL);

			pActiveUniqueClustersSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t) * (NUM_CLUSTERS + 1), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 2);
			pActiveUniqueClustersSSBO->Assign(NULL);

			pLightIndexSSBO = pResourceManager->CreateBuffer(sizeof(uint32_t) * (NUM_CLUSTERS * MAX_LIGHTS_PER_TILE + 1), BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 4);
			pLightIndexSSBO->Assign(NULL);

			pLightGridSSBO = pResourceManager->CreateBuffer(sizeof(LightGrid) * NUM_CLUSTERS, BufferBindingTarget::B_SHADER_STORAGE, MemoryUsage::MU_STATIC_COPY, 5);
			pLightGridSSBO->Assign(NULL);

			camera.SetUserData("ClusterSSBO", pClusterSSBO);
			camera.SetUserData("ActiveClustersSSBO", pActiveClustersSSBO);
			camera.SetUserData("ActiveUniqueClustersSSBO", pActiveUniqueClustersSSBO);
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
		Buffer* pActiveClustersSSBO = nullptr;
		Buffer* pActiveUniqueClustersSSBO = nullptr;
		Buffer* pLightIndexSSBO = nullptr;
		Buffer* pLightGridSSBO = nullptr;
		if (!camera.GetUserData("ClusterSSBO", pClusterSSBO)) return;
		if (!camera.GetUserData("ActiveClustersSSBO", pActiveClustersSSBO)) return;
		if (!camera.GetUserData("ActiveUniqueClustersSSBO", pActiveUniqueClustersSSBO)) return;
		if (!camera.GetUserData("LightIndexSSBO", pLightIndexSSBO)) return;
		if (!camera.GetUserData("LightGridSSBO", pLightGridSSBO)) return;

		//m_pMarkActiveClustersMaterial->Use();
		//pActiveClustersSSBO->Bind();
		//m_pMarkActiveClustersMaterial->SetFloat("zNear", camera.GetNear());
		//m_pMarkActiveClustersMaterial->SetFloat("zFar", camera.GetFar());
		//m_pMarkActiveClustersMaterial->SetUInt("tileSizeInPx", resolution.x / gridSize.x);
		//m_pMarkActiveClustersMaterial->SetUVec3("numClusters", gridSize);
		//m_pMarkActiveClustersMaterial->SetTexture("Depth", pDepthTexture);
		//pGraphics->DispatchCompute(resolution.x, resolution.y, 1);
		//pActiveClustersSSBO->Unbind();

		//m_pCompactClustersMaterial->Use();
		//pActiveClustersSSBO->Bind();
		//pActiveUniqueClustersSSBO->Bind();
		//pGraphics->DispatchCompute(NUM_CLUSTERS, 1, 1);
		//pActiveClustersSSBO->Unbind();
		//pActiveUniqueClustersSSBO->Unbind();

		const uint32_t count = (uint32_t)std::fmin(lights.size(), MAX_LIGHTS);
		m_pLightsSSBO->Assign(lights.data(), 0, count*sizeof(LightData));

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
		pGraphics->DispatchCompute(1, 1, 6);
		pClusterSSBO->Unbind();
		m_pScreenToViewSSBO->Unbind();
		m_pLightsSSBO->Unbind();
		pLightIndexSSBO->Unbind();
		pLightGridSSBO->Unbind();
	}

	void ClusteredRendererModule::LoadSettings(ModuleSettings& settings)
	{
		RendererModule::LoadSettings(settings);
		settings.RegisterAssetReference<PipelineData>("Screen Pipeline", 20);
		settings.RegisterAssetReference<PipelineData>("SSAO Prepass Pipeline", 21);
		settings.RegisterAssetReference<PipelineData>("SSAO Blur Pipeline", 22);
		settings.RegisterAssetReference<PipelineData>("Text Pipeline", 23);
		settings.RegisterAssetReference<PipelineData>("Display Copy Pipeline", 30);
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
}
