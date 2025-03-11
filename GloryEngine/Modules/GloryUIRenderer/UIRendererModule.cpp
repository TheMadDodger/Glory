#include "UIRendererModule.h"
#include "UIComponents.h"
#include "UIDocument.h"
#include "UIDocumentData.h"
#include "UIRenderSystem.h"
#include "UISystems.h"
#include "Constraints.h"

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
		references.push_back(settings.Value<uint64_t>("UI Prepass Pipeline"));
		references.push_back(settings.Value<uint64_t>("UI Text Prepass Pipeline"));
		references.push_back(settings.Value<uint64_t>("UI Overlay Pipeline"));
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

	void UIRendererModule::DrawDocument(UIDocument* pDocument, const UIRenderData& data)
	{
		pDocument->m_Registry.SetUserData(pDocument);
		pDocument->m_pRenderer = this;
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();
		RenderTexture* pRenderTexture = pDocument->m_pUITexture;
		pDocument->m_Projection = glm::ortho(0.0f, float(data.m_Resolution.x), 0.0f, float(data.m_Resolution.y));
		pDocument->Update();

		pRenderTexture->BindForDraw();
		pGraphics->Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
		pDocument->Draw();
		pRenderTexture->UnBindForDraw();
	}

	MaterialData* UIRendererModule::PrepassMaterial()
	{
		return m_pUIPrepassMaterial;
	}

	MaterialData* UIRendererModule::TextPrepassMaterial()
	{
		return m_pUITextPrepassMaterial;
	}

	MeshData* UIRendererModule::GetImageMesh()
	{
		return m_pImageMesh.get();
	}

	UIDocument* UIRendererModule::FindDocument(UUID uuid)
	{
		auto& iter = m_Documents.find(uuid);
		if (iter == m_Documents.end()) return nullptr;
		return &iter->second;
	}

	void UIRendererModule::Initialize()
	{
		Reflect::SetReflectInstance(&m_pEngine->Reflection());
		Reflect::RegisterEnum<UITarget>();
		Reflect::RegisterEnum<ResolutionMode>();
		Reflect::RegisterType<XConstraint>();
		Reflect::RegisterType<YConstraint>();
		Reflect::RegisterType<WidthConstraint>();
		Reflect::RegisterType<HeightConstraint>();

		Reflect::RegisterType<UIRenderer>();
		Reflect::RegisterType<UITransform>();
		Reflect::RegisterType<UIImage>();
		Reflect::RegisterType<UIText>();
		Reflect::RegisterType<UIBox>();
		Reflect::RegisterType<UIInteraction>();

		Constraints::AddBuiltinConstraints();

		/* Register the renderer component using the main component types instance */
		Utils::ECS::ComponentTypes* pComponentTypes = m_pEngine->GetSceneManager()->ComponentTypesInstance();
		m_pEngine->GetSceneManager()->RegisterComponent<UIRenderer>();
		m_pEngine->GetResourceTypes().RegisterResource<UIDocumentData>("");
		pComponentTypes->RegisterInvokaction<UIRenderer>(Glory::Utils::ECS::InvocationType::Draw, UIRenderSystem::OnDraw);

		/* Register the UI components with a different component types instance */
		m_pComponentTypes = Utils::ECS::ComponentTypes::CreateInstance();
		m_pComponentTypes->RegisterComponent<UITransform>();
		m_pComponentTypes->RegisterComponent<UIImage>();
		m_pComponentTypes->RegisterComponent<UIText>();
		m_pComponentTypes->RegisterComponent<UIBox>();
		m_pComponentTypes->RegisterComponent<UIInteraction>();
		/* Transform */
		m_pComponentTypes->RegisterInvokaction<UITransform>(Glory::Utils::ECS::InvocationType::Update, UITransformSystem::OnUpdate);
		/* Image */
		m_pComponentTypes->RegisterInvokaction<UIImage>(Glory::Utils::ECS::InvocationType::Draw, UIImageSystem::OnDraw);
		/* Text */
		m_pComponentTypes->RegisterInvokaction<UIText>(Glory::Utils::ECS::InvocationType::Draw, UITextSystem::OnDraw);
		m_pComponentTypes->RegisterInvokaction<UIText>(Glory::Utils::ECS::InvocationType::OnDirty, UITextSystem::OnDirty);
		/* Box */
		m_pComponentTypes->RegisterInvokaction<UIBox>(Glory::Utils::ECS::InvocationType::Draw, UIBoxSystem::OnDraw);
		/* Interaction */
		m_pComponentTypes->RegisterInvokaction<UIInteraction>(Glory::Utils::ECS::InvocationType::Update, UIInteractionSystem::OnUpdate);

		RendererModule* pRenderer = m_pEngine->GetMainModule<RendererModule>();
		pRenderer->AddRenderPass(RenderPassType::RP_Prepass, { "UI Prepass", [this](CameraRef camera, const RenderFrame& frame) {
			UIPrepass(camera, frame);
		} });

		pRenderer->AddRenderPass(RenderPassType::RP_CameraPostpass, { "UI Overlay Pass", [this](CameraRef camera, const RenderFrame& frame) {
			UIOverlayPass(camera, frame);
		} });

		m_pImageMesh.reset(new MeshData(4, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }));

		const float xpos = 0.0f;
		const float ypos = 0.0f;

		const float w = 1.0f;
		const float h = 1.0f;

		const glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

		VertexPosColorTex vertices[4] = {
			{ { xpos, ypos + h, }, color, {0.0f, 0.0f}},
			{ { xpos, ypos, }, color, { 0.0f, 1.0f } },
			{ { xpos + w, ypos, }, color, { 1.0f, 1.0f } },
			{ { xpos + w, ypos + h, }, color, { 1.0f, 0.0f }, }
		};

		m_pImageMesh->AddVertex(reinterpret_cast<float*>(&vertices[0]));
		m_pImageMesh->AddVertex(reinterpret_cast<float*>(&vertices[1]));
		m_pImageMesh->AddVertex(reinterpret_cast<float*>(&vertices[2]));
		m_pImageMesh->AddVertex(reinterpret_cast<float*>(&vertices[3]));
		m_pImageMesh->AddFace(0, 1, 2, 3);
	}

	void UIRendererModule::PostInitialize()
	{
		const ModuleSettings& settings = Settings();
		const UUID uiPrepassPipeline = settings.Value<uint64_t>("UI Prepass Pipeline");
		const UUID uiTextPrepassPipeline = settings.Value<uint64_t>("UI Text Prepass Pipeline");
		const UUID uiOverlayPipeline = settings.Value<uint64_t>("UI Overlay Pipeline");

		m_pUIPrepassMaterial = new MaterialData();
		m_pUIPrepassMaterial->SetPipeline(uiPrepassPipeline);
		m_pUIPrepassMaterial->AddProperty("Color", "Color", ResourceTypes::GetHash<glm::vec4>(), sizeof(glm::vec4));
		m_pUIPrepassMaterial->AddProperty("HasTexture", "HasTexture", ResourceTypes::GetHash<glm::vec4>(), sizeof(glm::vec4));
		m_pUITextPrepassMaterial = new MaterialData();
		m_pUITextPrepassMaterial->SetPipeline(uiTextPrepassPipeline);
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

		for (auto& data : m_Frame)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(data.m_DocumentID);
			if (!pResource) continue;
			UIDocumentData* pDocument = static_cast<UIDocumentData*>(pResource);

			UIDocument& document = GetDocument(data, pDocument);
			RenderTexture* pRenderTexture = document.m_pUITexture;
			document.m_Projection = glm::ortho(0.0f, float(data.m_Resolution.x), 0.0f, float(data.m_Resolution.y));
			document.m_CursorPos = data.m_CursorPos;
			document.m_CursorDown = data.m_CursorDown;
			document.Update();

			pRenderTexture->BindForDraw();
			pGraphics->Clear({ 0.0f, 0.0f, 0.0f, 0.0f });
			document.Draw();
			pRenderTexture->UnBindForDraw();
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
		settings.RegisterAssetReference<PipelineData>("UI Text Prepass Pipeline", 107);
		settings.RegisterAssetReference<PipelineData>("UI Overlay Pipeline", 105);
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

			newDocument.m_SceneID = data.m_SceneID;
			newDocument.m_ObjectID = data.m_ObjectID;
			newDocument.m_pUITexture = pResourceManager->CreateRenderTexture(uiTextureInfo);
			newDocument.m_pRenderer = this;
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
			newDocument.m_SceneID = data.m_SceneID;
			newDocument.m_ObjectID = data.m_ObjectID;
			newDocument.m_pUITexture = pUITexture;
			newDocument.m_pRenderer = this;
			return newDocument;
		}
		return document;
	}
}
