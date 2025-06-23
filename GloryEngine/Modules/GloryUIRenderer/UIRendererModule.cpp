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
#include <LocalizeModuleBase.h>

#include <DistributedRandom.h>

#include <EntityRegistry.h>
#include <GloryECS/ComponentTypes.h>
#include <Reflection.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <PropertyFlags.h>

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

		std::vector<UUID> newReferences;
		newReferences.push_back(settings.Value<uint64_t>("UI Prepass Stencil Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("UI Prepass Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("UI Text Prepass Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("UI Overlay Pipeline"));

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

	const std::type_info& UIRendererModule::GetModuleType()
	{
		return typeid(UIRendererModule);
	}

	void UIRendererModule::Submit(UIRenderData&& data)
	{
		m_Frame.push_back(std::move(data));
	}

	void UIRendererModule::Create(const UIRenderData& data, UIDocumentData* pDocument)
	{
		GetDocument(data, pDocument, true);
	}

	void UIRendererModule::DrawDocument(UIDocument* pDocument, const UIRenderData& data)
	{
		pDocument->m_Registry.SetUserData(pDocument);
		pDocument->m_pRenderer = this;
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();
		RenderTexture* pRenderTexture = pDocument->m_pUITexture;
		pDocument->m_Projection = glm::ortho(0.0f, float(data.m_Resolution.x), 0.0f, float(data.m_Resolution.y));
		pDocument->m_InputEnabled = data.m_InputEnabled;
		pDocument->Update();

		pRenderTexture->BindForDraw();
		pDocument->Draw(pGraphics, { 0.0f, 0.0f, 0.0f, 1.0f });
		pRenderTexture->UnBindForDraw();
	}

	MaterialData* UIRendererModule::PrepassStencilMaterial()
	{
		return m_pUIPrepassStencilMaterial;
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
		Reflect::RegisterEnum<ScrollMode>();
		Reflect::RegisterEnum<ScrollEdgeMode>();
		Reflect::RegisterType<XConstraint>();
		Reflect::RegisterType<YConstraint>();
		Reflect::RegisterType<WidthConstraint>();
		Reflect::RegisterType<HeightConstraint>();

		Reflect::RegisterType<UIRenderer>();
		Reflect::RegisterType<UITransform>();
		const FieldData* pColorField = Reflect::RegisterType<UIImage>()->GetFieldData("m_Color");
		Reflect::SetFieldFlags(pColorField, PropertyFlags::Color);
		const TypeData* pTextType = Reflect::RegisterType<UIText>();
		pColorField = pTextType->GetFieldData("m_Color");
		const FieldData* pTextField = pTextType->GetFieldData("m_Text");
		Reflect::SetFieldFlags(pColorField, PropertyFlags::Color);
		Reflect::SetFieldFlags(pTextField, PropertyFlags::AreaText);
		pColorField = Reflect::RegisterType<UIBox>()->GetFieldData("m_Color");
		Reflect::SetFieldFlags(pColorField, PropertyFlags::Color);
		Reflect::RegisterType<UIInteraction>();
		Reflect::RegisterType<UIPanel>();
		Reflect::RegisterType<UIVerticalContainer>();
		Reflect::RegisterType<UIScrollView>();

		Constraints::AddBuiltinConstraints();

		/* Register the renderer component using the main component types instance */
		Utils::ECS::ComponentTypes* pComponentTypes = m_pEngine->GetSceneManager()->ComponentTypesInstance();
		m_pEngine->GetSceneManager()->RegisterComponent<UIRenderer>();
		m_pEngine->GetResourceTypes().RegisterResource<UIDocumentData>("");
		pComponentTypes->RegisterInvokaction<UIRenderer>(Glory::Utils::ECS::InvocationType::Draw, UIRenderSystem::OnDraw);
		pComponentTypes->RegisterInvokaction<UIRenderer>(Glory::Utils::ECS::InvocationType::OnValidate, UIRenderSystem::OnValidate);
		pComponentTypes->RegisterInvokaction<UIRenderer>(Glory::Utils::ECS::InvocationType::Start, UIRenderSystem::OnStart);
		pComponentTypes->RegisterReferencesCallback<UIRenderer>(UIRenderSystem::GetReferences);

		/* Register the UI components with a different component types instance */
		m_pComponentTypes = Utils::ECS::ComponentTypes::CreateInstance();
		m_pComponentTypes->RegisterComponent<UITransform>();
		m_pComponentTypes->RegisterComponent<UIImage>();
		m_pComponentTypes->RegisterComponent<UIText>();
		m_pComponentTypes->RegisterComponent<UIBox>();
		m_pComponentTypes->RegisterComponent<UIInteraction>();
		m_pComponentTypes->RegisterComponent<UIPanel>();
		m_pComponentTypes->RegisterComponent<UIVerticalContainer>();
		m_pComponentTypes->RegisterComponent<UIScrollView>();
		/* Transform */
		m_pComponentTypes->RegisterInvokaction<UITransform>(Glory::Utils::ECS::InvocationType::PostUpdate, UITransformSystem::OnPostUpdate);
		/* Image */
		m_pComponentTypes->RegisterInvokaction<UIImage>(Glory::Utils::ECS::InvocationType::Draw, UIImageSystem::OnDraw);
		m_pComponentTypes->RegisterReferencesCallback<UIImage>(UIImageSystem::GetReferences);
		/* Text */
		m_pComponentTypes->RegisterInvokaction<UIText>(Glory::Utils::ECS::InvocationType::Start, UITextSystem::OnStart);
		m_pComponentTypes->RegisterInvokaction<UIText>(Glory::Utils::ECS::InvocationType::Draw, UITextSystem::OnDraw);
		m_pComponentTypes->RegisterInvokaction<UIText>(Glory::Utils::ECS::InvocationType::OnDirty, UITextSystem::OnDirty);
		m_pComponentTypes->RegisterReferencesCallback<UIText>(UITextSystem::GetReferences);
		/* Box */
		m_pComponentTypes->RegisterInvokaction<UIBox>(Glory::Utils::ECS::InvocationType::Draw, UIBoxSystem::OnDraw);
		/* Interaction */
		m_pComponentTypes->RegisterInvokaction<UIInteraction>(Glory::Utils::ECS::InvocationType::Update, UIInteractionSystem::OnUpdate);
		/* Panel */
		m_pComponentTypes->RegisterInvokaction<UIPanel>(Glory::Utils::ECS::InvocationType::Draw, UIPanelSystem::OnDraw);
		m_pComponentTypes->RegisterInvokaction<UIPanel>(Glory::Utils::ECS::InvocationType::PostDraw, UIPanelSystem::OnPostDraw);
		/* Vertical Container */
		m_pComponentTypes->RegisterInvokaction<UIVerticalContainer>(Glory::Utils::ECS::InvocationType::OnDirty, UIVerticalContainerSystem::OnDirty);
		m_pComponentTypes->RegisterInvokaction<UIVerticalContainer>(Glory::Utils::ECS::InvocationType::PreUpdate, UIVerticalContainerSystem::OnPreUpdate);
		m_pComponentTypes->RegisterInvokaction<UIVerticalContainer>(Glory::Utils::ECS::InvocationType::Update, UIVerticalContainerSystem::OnUpdate);
		/* Scroll View */
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::Start, UIScrollViewSystem::OnStart);
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::OnValidate, UIScrollViewSystem::OnValidate);
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::OnDirty, UIScrollViewSystem::OnDirty);
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::PreUpdate, UIScrollViewSystem::OnPreUpdate);
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::Update, UIScrollViewSystem::OnUpdate);

		RendererModule* pRenderer = m_pEngine->GetMainModule<RendererModule>();
		pRenderer->AddRenderPass(RenderPassType::RP_Prepass, { "UI Prepass", [this](CameraRef camera, const RenderFrame& frame) {
			UIPrepass(camera, frame);
		} });

		pRenderer->AddRenderPass(RenderPassType::RP_CameraPostpass, { "UI Overlay Pass", [this](CameraRef camera, const RenderFrame& frame) {
			UIOverlayPass(camera, frame);
		} });

		pRenderer->AddRenderPass(RenderPassType::RP_Objectpass, { "UI Worldspace Quad Pass", [this](CameraRef camera, const RenderFrame& frame) {
			UIWorldSpaceQuadPass(camera, frame);
		} });

		LocalizeModuleBase* pLocalize = m_pEngine->GetOptionalModule<LocalizeModuleBase>();
		if (pLocalize)
		{
			pLocalize->OnLanguageChanged = [this]() {
				for (auto& iter : m_Documents)
				{
					iter.second.Registry().InvokeAll<UIText>(Utils::ECS::InvocationType::Start);
				}
			};
		}

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
		const UUID uiPrepassStencilPipeline = settings.Value<uint64_t>("UI Prepass Stencil Pipeline");
		const UUID uiPrepassPipeline = settings.Value<uint64_t>("UI Prepass Pipeline");
		const UUID uiTextPrepassPipeline = settings.Value<uint64_t>("UI Text Prepass Pipeline");
		const UUID uiOverlayPipeline = settings.Value<uint64_t>("UI Overlay Pipeline");

		m_pUIPrepassStencilMaterial = new MaterialData();
		m_pUIPrepassStencilMaterial->SetPipeline(uiPrepassStencilPipeline);
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
		delete m_pUIPrepassStencilMaterial;
		m_pUIPrepassStencilMaterial = nullptr;
		
		delete m_pUIPrepassMaterial;
		m_pUIPrepassMaterial = nullptr;
		
		delete m_pUITextPrepassMaterial;
		m_pUITextPrepassMaterial = nullptr;

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
			document.m_CursorScrollDelta = data.m_CursorScrollDelta;
			document.m_CursorDown = data.m_CursorDown;
			document.m_InputEnabled = data.m_InputEnabled;
			document.Update();

			document.Draw(pGraphics);
		}
	}

	void UIRendererModule::UIWorldSpaceQuadPass(CameraRef camera, const RenderFrame&)
	{
		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();

		for (auto& data : m_Frame)
		{
			if (data.m_Target != UITarget::WorldSpaceQuad) continue;

			/* Get document */
			auto& iter = m_Documents.find(data.m_ObjectID);
			if (iter == m_Documents.end()) continue;
			UIDocument& document = iter->second;
			RenderTexture* pDocumentTexture = document.m_pUITexture;

			MaterialData* pMaterialData = m_pEngine->GetMaterialManager().GetMaterial(data.m_MaterialID);
			if (!pMaterialData) return;
			Material* pMaterial = pGraphics->UseMaterial(pMaterialData);
			if (!pMaterial) return;

			ObjectData object;
			object.Model = data.m_WorldTransform;
			object.View = camera.GetView();
			object.Projection = camera.GetProjection();
			object.ObjectID = data.m_ObjectID;
			object.SceneID = data.m_SceneID;

			MeshData* pMeshData = GetDocumentQuadMesh(data);
			pMaterial->SetSamplers(m_pEngine);
			pMaterial->ResetTextureCounter();
			pMaterial->SetTexture("texSampler", pDocumentTexture->GetTextureAttachment(0));
			pMaterial->SetPropertiesBuffer(m_pEngine);
			pMaterial->SetObjectData(object);
			pGraphics->DrawMesh(pMeshData, 0, pMeshData->VertexCount());
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
			if (data.m_Target != UITarget::CameraOverlay || camera.GetUUID() != data.m_TargetCamera) continue;

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
		settings.RegisterAssetReference<PipelineData>("UI Prepass Stencil Pipeline", 109);
		settings.RegisterAssetReference<PipelineData>("UI Prepass Pipeline", 102);
		settings.RegisterAssetReference<PipelineData>("UI Text Prepass Pipeline", 107);
		settings.RegisterAssetReference<PipelineData>("UI Overlay Pipeline", 105);
	}

	UIDocument& UIRendererModule::GetDocument(const UIRenderData& data, UIDocumentData* pDocument, bool forceCreate)
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
			uiTextureInfo.HasStencil = true;
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
		if (width != data.m_Resolution.x || height != data.m_Resolution.y)
		{
			document.m_pUITexture->Resize(data.m_Resolution.x, data.m_Resolution.y);
			document.m_DrawIsDirty = true;
		}

		if (document.m_OriginalDocumentID != pDocument->GetUUID() || forceCreate)
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

	MeshData* UIRendererModule::GetDocumentQuadMesh(const UIRenderData& data)
	{
		bool createMesh = data.m_WorldDirty;

		auto iter = m_pDocumentQuads.find(data.m_ObjectID);
		if (iter == m_pDocumentQuads.end())
		{
			iter = m_pDocumentQuads.emplace(data.m_ObjectID,
				new MeshData(4, sizeof(DefaultVertex3D), {AttributeType::Float3, AttributeType::Float3,
					AttributeType::Float3, AttributeType::Float3, AttributeType::Float2, AttributeType::Float4 })
			).first;

			createMesh = true;
		}

		if (createMesh)
		{
			iter->second->ClearIndices();
			iter->second->ClearVertices();

			const float xpos = -(data.m_WorldSize.x/2.0f);
			const float ypos = -(data.m_WorldSize.y/2.0f);

			const float w = data.m_WorldSize.x;
			const float h = data.m_WorldSize.y;

			const glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
			const glm::vec3 normal{ 0.0f, 0.0f, 1.0f };
			const glm::vec3 tangent{ 1.0f, 0.0f, 0.0f };
			const glm::vec3 bitangent{ 0.0f, 1.0f, 0.0f };

			DefaultVertex3D vertices[4] = {
				{ { xpos, ypos + h, 0.0f }, normal, tangent, bitangent, { 0.0f, 1.0f }, color},
				{ { xpos, ypos, 0.0f }, normal, tangent, bitangent, { 0.0f, 0.0f }, color },
				{ { xpos + w, ypos, 0.0f }, normal, tangent, bitangent, { 1.0f, 0.0f }, color },
				{ { xpos + w, ypos + h, 0.0f }, normal, tangent, bitangent, { 1.0f, 1.0f }, color }
			};

			iter->second->AddVertex(reinterpret_cast<float*>(&vertices[0]));
			iter->second->AddVertex(reinterpret_cast<float*>(&vertices[1]));
			iter->second->AddVertex(reinterpret_cast<float*>(&vertices[2]));
			iter->second->AddVertex(reinterpret_cast<float*>(&vertices[3]));
			iter->second->AddFace(0, 1, 2, 3);
			iter->second->SetDirty(true);
		}

		return iter->second.get();
	}
}
