#include "UIRendererModule.h"
#include "UIComponents.h"
#include "UIDocument.h"
#include "UIDocumentData.h"
#include "UIRenderSystem.h"

#include <AssetManager.h>
#include <MaterialManager.h>
#include <Engine.h>
#include <GraphicsModule.h>
#include <GPUResourceManager.h>
#include <CameraManager.h>
#include <InternalMaterial.h>
#include <InternalPipeline.h>
#include <FontData.h>
#include <FontDataStructs.h>
#include <Material.h>
#include <MaterialData.h>
#include <SceneManager.h>

#include <DistributedRandom.h>

#include <EntityRegistry.h>
#include <GloryECS/ComponentTypes.h>
#include <Reflection.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(UIRendererModule);

	UIRendererModule::UIRendererModule()
	{
	}

	UIRendererModule::~UIRendererModule()
	{
	}

	void UIRendererModule::CollectReferences(std::vector<UUID>& references)
	{
		ModuleSettings& settings = Settings();

		const size_t start = references.size();
		references.push_back(settings.Value<uint64_t>("UI Pipeline"));
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

	const std::type_info& UIRendererModule::GetModuleType()
	{
		return typeid(UIRendererModule);
	}

	void UIRendererModule::Submit(UIRenderData&& data)
	{
		m_Frame.push_back(std::move(data));
	}

	void GenerateTextMesh(MeshData* pMesh, FontData* pFontData, const TextRenderData& renderData)
	{
		const std::string_view text = renderData.m_Text;
		const glm::vec4& color = renderData.m_Color;
		const float scale = renderData.m_Scale;
		const Alignment alignment = renderData.m_Alignment;
		const float textWrap = renderData.m_TextWrap * scale * pFontData->FontHeight();

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
				const float advance = (glyph->Advance >> 6) * scale;
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

	void UIRendererModule::Initialize()
	{
		Reflect::SetReflectInstance(&m_pEngine->Reflection());
		Reflect::RegisterEnum<UITarget>();
		Reflect::RegisterEnum<ResolutionMode>();

		Reflect::RegisterType<UIRenderer>();
		Reflect::RegisterType<UITransform>();
		Reflect::RegisterType<UIText>();

		/* Register the renderer component using the main component types instance */
		Utils::ECS::ComponentTypes* pComponentTypes = m_pEngine->GetSceneManager()->ComponentTypesInstance();
		m_pEngine->GetSceneManager()->RegisterComponent<UIRenderer>();
		m_pEngine->GetResourceTypes().RegisterResource<UIDocumentData>("");
		pComponentTypes->RegisterInvokaction<UIRenderer>(Glory::Utils::ECS::InvocationType::Draw, UIRenderSystem::OnDraw);

		/* Register the UI components with a different component types instance */
		m_pComponentTypes = Utils::ECS::ComponentTypes::CreateInstance();
		m_pComponentTypes->RegisterComponent<UITransform>();
		m_pComponentTypes->RegisterComponent<UIText>();

		RendererModule* pRenderer = m_pEngine->GetMainModule<RendererModule>();
		pRenderer->AddRenderPass(RenderPassType::RP_Prepass, { "UI Prepass", [this](CameraRef camera, const RenderFrame& frame) {
			UIPrepass(camera, frame);
		} });

		pRenderer->AddRenderPass(RenderPassType::RP_CameraPostpass, { "UI Overlay Pass", [this](CameraRef camera, const RenderFrame& frame) {
			UIOverlayPass(camera, frame);
		} });
	}

	void UIRendererModule::PostInitialize()
	{
		const ModuleSettings& settings = Settings();
		const UUID uiPrepassPipeline = settings.Value<uint64_t>("UI Prepass Pipeline");
		const UUID uiOverlayPipeline = settings.Value<uint64_t>("UI Overlay Pipeline");

		m_pUIPrepassMaterial = new MaterialData();
		m_pUIPrepassMaterial->SetPipeline(uiPrepassPipeline);
		m_pUIOverlayMaterial = new MaterialData();
		m_pUIOverlayMaterial->SetPipeline(uiOverlayPipeline);
	}

	void UIRendererModule::Update()
	{
		m_Frame.clear();
	}

	void UIRendererModule::Cleanup()
	{
		delete m_pUIPrepassMaterial;
		m_pUIPrepassMaterial = nullptr;

		delete m_pUIOverlayMaterial;
		m_pUIOverlayMaterial = nullptr;

		Utils::ECS::ComponentTypes::DestroyInstance();
	}

	void UIRendererModule::UIPrepass(CameraRef, const RenderFrame&)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		ModuleSettings& settings = Settings();
		const UUID fontID = settings.Value<uint64_t>("Font");
		if (fontID == 0) return;
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(fontID);
		if (!pResource) return;
		FontData* pFont = static_cast<FontData*>(pResource);

		for (auto& data : m_Frame)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(data.m_DocumentID);
			if (!pResource) continue;
			UIDocumentData* pDocument = static_cast<UIDocumentData*>(pResource);

			UIDocument& document = GetDocument(data, pDocument);
			RenderTexture* pRenderTexture = document.m_pUITexture;

			Utils::ECS::EntityRegistry& registry = document.m_Registry;
			Utils::ECS::TypeView<UIText>* pTextView = registry.GetTypeView<UIText>();

			for (size_t i = 0; i < pTextView->Size(); ++i)
			{
				Utils::ECS::EntityID entity = pTextView->EntityAt(i);
				const UIText& text = pTextView->Get(entity);
				const UITransform& transform = registry.GetComponent<UITransform>(entity);

				TextRenderData textData;
				textData.m_Alignment = Alignment::Left;
				textData.m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				textData.m_Scale = 1.0f;
				textData.m_Text = text.m_Text;
				textData.m_ObjectID = entity;
				textData.m_TextDirty = false;

				auto iter = m_pTextMeshes.find(textData.m_ObjectID);
				const bool exists = iter != m_pTextMeshes.end();
				if (!exists)
				{
					MeshData* pMesh = new MeshData(textData.m_Text.size()*4, sizeof(VertexPosColorTex),
						{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
					iter = m_pTextMeshes.emplace(textData.m_ObjectID, pMesh).first;
				}

				if (textData.m_TextDirty || !exists)
					GenerateTextMesh(iter->second.get(), pFont, textData);

				Mesh* pMesh = pResourceManager->CreateMesh(iter->second.get());

				ObjectData object;
				object.Model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(transform.m_Rect.x, transform.m_Rect.y, 1.0f));
				object.Projection = glm::ortho(0.0f, float(data.m_Resolution.x), 0.0f, float(data.m_Resolution.y));

				Material* pMaterial = pGraphics->UseMaterial(m_pUIPrepassMaterial);

				pMaterial->SetProperties(m_pEngine);
				pMaterial->SetObjectData(object);

				InternalTexture* pTextureData = pFont->GetGlyphTexture();
				if (!pTextureData) return;

				Texture* pTexture = pResourceManager->CreateTexture((TextureData*)pTextureData);
				if (pTexture) pMaterial->SetTexture("textSampler", pTexture);

				pRenderTexture->BindForDraw();
				pGraphics->Clear({ 0.0f, 0.0f, 0.0f, 0.0f });
				pGraphics->DrawMesh(pMesh, 0, pMesh->GetVertexCount());
				pRenderTexture->UnBindForDraw();
			}
		}
	}

	void UIRendererModule::UIOverlayPass(CameraRef camera, const RenderFrame&)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		RenderTexture* pCameraTexture = camera.GetOutputTexture();
		RenderTexture* pOutputTexture = camera.GetSecondaryOutputTexture();
		uint32_t width, height;
		pOutputTexture->GetDimensions(width, height);

		pGraphics->EnableDepthTest(false);
		pGraphics->SetViewport(0, 0, width, height);

		for (auto& data : m_Frame)
		{
			if (camera.GetUUID() != data.m_TargetCamera) continue;

			/* Get document */
			auto& iter = m_Documents.find(data.m_ObjectID);
			if (iter == m_Documents.end()) continue;
			UIDocument& document = iter->second;
			RenderTexture* pDocumentTexture = document.m_pUITexture;

			pCameraTexture = camera.GetOutputTexture();
			pOutputTexture = camera.GetSecondaryOutputTexture();

			/* Render to the output texture */
			pOutputTexture->BindForDraw();

			/* Use overlay material */
			Material* pMaterial = pGraphics->UseMaterial(m_pUIOverlayMaterial);

			/* Bind camera texture and document texture */
			pCameraTexture->BindAll(pMaterial);
			pDocumentTexture->BindAll(pMaterial);

			/* Draw the screen quad */
			pGraphics->DrawScreenQuad();

			pOutputTexture->UnBindForDraw();

			pGraphics->UseMaterial(nullptr);

			/* Swap the cameras textures for the next pass */
			camera.Swap();
		}

		/* Reset render textures and materials */
		pGraphics->UseMaterial(nullptr);
		pGraphics->EnableDepthTest(true);
	}

	void UIRendererModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterAssetReference<PipelineData>("UI Prepass Pipeline", 102);
		settings.RegisterAssetReference<PipelineData>("UI Overlay Pipeline", 105);
		settings.RegisterAssetReference<FontData>("Font", 0);
	}

	UIDocument& UIRendererModule::GetDocument(const UIRenderData& data, UIDocumentData* pDocument)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();

		const UUID id = data.m_ObjectID;

		auto& iter = m_Documents.find(id);
		if (iter == m_Documents.end())
		{
			m_Documents.emplace(id, pDocument);
			UIDocument& newDocument = m_Documents.at(id);

			RenderTextureCreateInfo uiTextureInfo;
			uiTextureInfo.HasDepth = false;
			uiTextureInfo.Width = uint32_t(data.m_Resolution.x);
			uiTextureInfo.Height = uint32_t(data.m_Resolution.y);
			uiTextureInfo.Attachments.push_back(Attachment("UIColor", PixelFormat::PF_RGBA, PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
			uiTextureInfo.HasDepth = false;

			newDocument.m_pUITexture = pResourceManager->CreateRenderTexture(uiTextureInfo);
			return newDocument;
		}

		UIDocument& document = iter->second;
		uint32_t width, height;
		document.m_pUITexture->GetDimensions(width, height);
		if (width != data.m_Resolution.x || width != data.m_Resolution.y)
		{
			document.m_pUITexture->Resize(data.m_Resolution.x, data.m_Resolution.y);
		}

		if (document.m_OriginalDocumentID != pDocument->GetUUID())
		{
			RenderTexture* pUITexture = document.m_pUITexture;
			m_Documents.erase(iter);
			m_Documents.emplace(id, pDocument);
			UIDocument& newDocument = m_Documents.at(id);
			newDocument.m_pUITexture = pUITexture;
			return newDocument;
		}
		return document;
	}
}
