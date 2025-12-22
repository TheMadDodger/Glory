#include "UIRendererModule.h"
#include "UIComponents.h"
#include "UIDocument.h"
#include "UIDocumentData.h"
#include "UIRenderSystem.h"
#include "UISystems.h"
#include "Constraints.h"

#include <AssetManager.h>
#include <PipelineManager.h>
#include <MaterialManager.h>
#include <Engine.h>
#include <GraphicsDevice.h>
#include <CameraManager.h>
#include <InternalMaterial.h>
#include <InternalPipeline.h>
#include <FontData.h>
#include <FontDataStructs.h>
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
	struct UIConstants
	{
		glm::mat4 Projection;
		uint32_t ObjectIndex;
		uint32_t ColorIndex;
		uint32_t HasTexture;
	};

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

	void CalculateProjection(glm::mat4& projection, GraphicsDevice* pDevice, float x, float y, float width, float height)
	{
		projection = glm::ortho(x, width, y, height);
	}

	void UIRendererModule::DrawDocument(GraphicsDevice* pDevice, CommandBufferHandle commandBuffer,
		uint32_t frameIndex, UIDocument* pDocument, const UIRenderData& data)
	{
		pDocument->m_Registry.SetUserData(pDocument);
		pDocument->m_pRenderer = this;
		if (!pDevice) return;

		CalculateProjection(pDocument->m_Projection, pDevice, 0.0f, 0.0f, float(data.m_Resolution.x), float(data.m_Resolution.y));
		pDocument->m_InputEnabled = data.m_InputEnabled;
		pDocument->Update();

		if (!pDocument->m_DrawIsDirty.IsSet(frameIndex)) return;
		pDocument->Draw();
		pDocument->m_DrawIsDirty.Set(frameIndex, false);

		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		AssetManager& assets = m_pEngine->GetAssetManager();
		const ModuleSettings& settings = Settings();

		/* Prepare pipelines */
		if (!m_UIBuffersLayout)
		{
			DescriptorSetLayoutInfo bufferSetLayoutInfo;
			bufferSetLayoutInfo.m_PushConstantRange.m_Offset = 0;
			bufferSetLayoutInfo.m_PushConstantRange.m_ShaderStages = ShaderTypeFlag(STF_Vertex | STF_Fragment);
			bufferSetLayoutInfo.m_PushConstantRange.m_Size = sizeof(UIConstants);
			bufferSetLayoutInfo.m_Buffers.resize(2);
			bufferSetLayoutInfo.m_Buffers[0].m_BindingIndex = 1;
			bufferSetLayoutInfo.m_Buffers[0].m_ShaderStages = STF_Vertex;
			bufferSetLayoutInfo.m_Buffers[0].m_Type = BT_Storage;
			bufferSetLayoutInfo.m_Buffers[1].m_BindingIndex = 2;
			bufferSetLayoutInfo.m_Buffers[1].m_ShaderStages = STF_Vertex;
			bufferSetLayoutInfo.m_Buffers[1].m_Type = BT_Storage;
			m_UIBuffersLayout = pDevice->CreateDescriptorSetLayout(std::move(bufferSetLayoutInfo));
		}

		if (!m_UISamplerLayout)
		{
			DescriptorSetLayoutInfo samplerSetLayoutInfo;
			samplerSetLayoutInfo.m_SamplerNames = { "Color" };
			samplerSetLayoutInfo.m_Samplers.resize(1);
			samplerSetLayoutInfo.m_Samplers[0].m_BindingIndex = 0;
			samplerSetLayoutInfo.m_Samplers[0].m_ShaderStages = STF_Fragment;
			m_UISamplerLayout = pDevice->CreateDescriptorSetLayout(std::move(samplerSetLayoutInfo));
		}

		PipelineData* pPipeline = pipelines.GetPipelineData(settings.Value<uint64_t>("UI Prepass Pipeline"));
		PipelineHandle uiPipeline = pDevice->AcquireCachedPipeline(pDocument->m_UIPasses[0], pPipeline,
			{ m_UIBuffersLayout, m_UISamplerLayout }, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
		pPipeline = pipelines.GetPipelineData(settings.Value<uint64_t>("UI Text Prepass Pipeline"));
		PipelineHandle uiTextPipeline = pDevice->AcquireCachedPipeline(pDocument->m_UIPasses[0], pPipeline,
			{ m_UIBuffersLayout, m_UISamplerLayout }, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });

		/* Prepare data */
		auto iter = m_BatchDatas.find(pDocument->m_ObjectID);
		if (iter == m_BatchDatas.end())
			iter = m_BatchDatas.emplace(pDocument->m_ObjectID, UIBatchData()).first;

		UIBatchData& batchData = iter->second;
		if (batchData.m_Worlds->size() < pDocument->m_UIBatch.m_Worlds.size())
			batchData.m_Worlds.resize(pDocument->m_UIBatch.m_Worlds.size());
		if (batchData.m_Colors->size() < pDocument->m_UIBatch.m_UniqueColors.size())
			batchData.m_Colors.resize(pDocument->m_UIBatch.m_UniqueColors.size());

		for (size_t i = 0; i < pDocument->m_UIBatch.m_Worlds.size(); ++i)
		{
			if (pDocument->m_UIBatch.m_Worlds[i] == batchData.m_Worlds.m_Data[i]) continue;
			batchData.m_Worlds.m_Data[i] = pDocument->m_UIBatch.m_Worlds[i];
			batchData.m_Worlds.SetDirty(i);
		}

		for (size_t i = 0; i < pDocument->m_UIBatch.m_UniqueColors.size(); ++i)
		{
			if (pDocument->m_UIBatch.m_UniqueColors[i] == batchData.m_Colors.m_Data[i]) continue;
			batchData.m_Colors.m_Data[i] = pDocument->m_UIBatch.m_UniqueColors[i];
			batchData.m_Colors.SetDirty(i);
		}

		if (!batchData.m_WorldsBuffers)
		{
			batchData.m_WorldsBuffers = pDevice->CreateBuffer(pDocument->m_UIBatch.m_Worlds.size()*sizeof(glm::mat4), BT_Storage, BF_Write);
			batchData.m_Worlds.SetDirty();
		}

		if (!batchData.m_ColorsBuffers)
		{
			batchData.m_ColorsBuffers = pDevice->CreateBuffer(pDocument->m_UIBatch.m_UniqueColors.size()*sizeof(glm::vec4), BT_Storage, BF_Write);
			batchData.m_Colors.SetDirty();
		}

		if (pDevice->BufferSize(batchData.m_WorldsBuffers) < batchData.m_Worlds.TotalByteSize())
			pDevice->ResizeBuffer(batchData.m_WorldsBuffers, batchData.m_Worlds.TotalByteSize());
		if (batchData.m_Worlds)
		{
			const size_t dirtySize = batchData.m_Worlds.DirtySize();
			pDevice->AssignBuffer(batchData.m_WorldsBuffers, batchData.m_Worlds.DirtyStart(),
				batchData.m_Worlds.m_DirtyRange.first*sizeof(glm::mat4), dirtySize*sizeof(glm::mat4));
		}

		if (pDevice->BufferSize(batchData.m_ColorsBuffers) < batchData.m_Colors.TotalByteSize())
			pDevice->ResizeBuffer(batchData.m_ColorsBuffers, batchData.m_Colors.TotalByteSize());
		if (batchData.m_Colors)
		{
			const size_t dirtySize = batchData.m_Colors.DirtySize();
			pDevice->AssignBuffer(batchData.m_ColorsBuffers, batchData.m_Colors.DirtyStart(),
				batchData.m_Colors.m_DirtyRange.first*sizeof(glm::vec4), dirtySize*sizeof(glm::vec4));
		}

		if (!batchData.m_BuffersSet)
		{
			DescriptorSetInfo setInfo;
			setInfo.m_Layout = m_UIBuffersLayout;
			setInfo.m_Buffers.resize(2);
			setInfo.m_Buffers[0].m_BufferHandle = batchData.m_WorldsBuffers;
			setInfo.m_Buffers[0].m_Offset = 0;
			setInfo.m_Buffers[0].m_Size = batchData.m_Worlds.TotalByteSize();
			setInfo.m_Buffers[1].m_BufferHandle = batchData.m_ColorsBuffers;
			setInfo.m_Buffers[1].m_Offset = 0;
			setInfo.m_Buffers[1].m_Size = batchData.m_Colors.TotalByteSize();
			batchData.m_BuffersSet = pDevice->CreateDescriptorSet(std::move(setInfo));
		}

		if (batchData.m_Worlds.SizeDirty() || batchData.m_Colors.SizeDirty())
		{
			DescriptorSetUpdateInfo dsWrite;
			dsWrite.m_Buffers.resize(2);
			dsWrite.m_Buffers[0].m_BufferHandle = batchData.m_WorldsBuffers;
			dsWrite.m_Buffers[0].m_DescriptorIndex = 0;
			dsWrite.m_Buffers[0].m_Offset = 0;
			dsWrite.m_Buffers[0].m_Size = batchData.m_Worlds.TotalByteSize();
			dsWrite.m_Buffers[1].m_BufferHandle = batchData.m_ColorsBuffers;
			dsWrite.m_Buffers[1].m_DescriptorIndex = 1;
			dsWrite.m_Buffers[1].m_Offset = 0;
			dsWrite.m_Buffers[1].m_Size = batchData.m_Colors.TotalByteSize();
			pDevice->UpdateDescriptorSet(batchData.m_BuffersSet, dsWrite);
		}

		if (batchData.m_TextureSets.size() < pDocument->m_UIBatch.m_TextureIDs.size())
		{
			batchData.m_TextureSets.resize(pDocument->m_UIBatch.m_TextureIDs.size(), nullptr);
			batchData.m_LastTextureIDs.resize(pDocument->m_UIBatch.m_TextureIDs.size(), 0ull);
		}

		for (size_t i = 0; i < batchData.m_TextureSets.size(); ++i)
		{
			UUID textureID = pDocument->m_UIBatch.m_TextureIDs[i];
			Resource* pTextureResource = textureID ? assets.FindResource(textureID) : nullptr;
			TextureData* pTexture = pTextureResource ? static_cast<TextureData*>(pTextureResource) : nullptr;
			TextureHandle texture = pDevice->AcquireCachedTexture(pTexture);
			if (!pTexture) textureID = 0;

			if (!batchData.m_TextureSets[i])
			{
				DescriptorSetInfo setInfo;
				setInfo.m_Layout = m_UISamplerLayout;
				setInfo.m_Samplers.resize(1);
				setInfo.m_Samplers[0].m_TextureHandle = texture;
				batchData.m_TextureSets[i] = pDevice->CreateDescriptorSet(std::move(setInfo));
				batchData.m_LastTextureIDs[i] = textureID;
			}

			if (batchData.m_LastTextureIDs[i] != textureID)
			{
				DescriptorSetUpdateInfo dsWrite;
				dsWrite.m_Samplers.resize(1);
				dsWrite.m_Samplers[0].m_DescriptorIndex = 0;
				dsWrite.m_Samplers[0].m_TextureHandle = texture;
				pDevice->UpdateDescriptorSet(batchData.m_TextureSets[i], dsWrite);
				batchData.m_LastTextureIDs[i] = textureID;
			}
		}

		m_ImageMesh = pDevice->AcquireCachedMesh(m_pImageMesh.get());

		RenderPassHandle renderPass = pDocument->m_UIPasses[frameIndex];
		pDevice->SetRenderPassClear(renderPass, { 0.0f, 0.0f, 0.0f, 1.0f });
		pDevice->BeginRenderPass(commandBuffer, renderPass);

		UIConstants constants;
		constants.Projection = pDocument->m_Projection;
		for (size_t i = 0; i < pDocument->m_UIBatch.m_Worlds.size(); ++i)
		{
			constants.ObjectIndex = i;
			constants.ColorIndex = pDocument->m_UIBatch.m_ColorIndices[i];
			constants.HasTexture = pDocument->m_UIBatch.m_TextureIDs[i] ? 1 : 0;

			PipelineHandle pipeline = pDocument->m_UIBatch.m_TextMeshes[i] ? uiTextPipeline : uiPipeline;
			pDevice->BeginPipeline(commandBuffer, pipeline);
			pDevice->PushConstants(commandBuffer, pipeline, 0, sizeof(UIConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
			pDevice->BindDescriptorSets(commandBuffer, pipeline, { batchData.m_BuffersSet, batchData.m_TextureSets[i] });
			pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(pDocument->m_Resolution.x), float(pDocument->m_Resolution.y));
			pDevice->SetScissor(commandBuffer, 0, 0, pDocument->m_Resolution.x, pDocument->m_Resolution.y);
			pDevice->DrawMesh(commandBuffer, m_ImageMesh);
			pDevice->EndPipeline(commandBuffer);
		}
		pDevice->EndRenderPass(commandBuffer);
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
		/* Scroll View */
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::Start, UIScrollViewSystem::OnStart);
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::OnValidate, UIScrollViewSystem::OnValidate);
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::OnDirty, UIScrollViewSystem::OnDirty);
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::PreUpdate, UIScrollViewSystem::OnPreUpdate);
		m_pComponentTypes->RegisterInvokaction<UIScrollView>(Glory::Utils::ECS::InvocationType::Update, UIScrollViewSystem::OnUpdate);

		//RendererModule* pRenderer = m_pEngine->GetMainModule<RendererModule>();
		//pRenderer->AddRenderPass(RenderPassType::RP_Prepass, { "UI Prepass", [this](uint32_t, RendererModule*) {
		//	UIPrepass();
		//} });
		//
		//pRenderer->AddRenderPass(RenderPassType::RP_CameraCompositePass, { "UI Overlay Pass", [this](uint32_t cameraIndex, RendererModule* pRenderer) {
		//	UIOverlayPass(cameraIndex, pRenderer);
		//} });
		//
		//pRenderer->AddRenderPass(RenderPassType::RP_ObjectPass, { "UI Worldspace Quad Pass", [this](uint32_t cameraIndex, RendererModule* pRenderer) {
		//	UIWorldSpaceQuadPass(cameraIndex, pRenderer);
		//} });

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

	void UIRendererModule::UIPrepass()
	{
		for (auto& data : m_Frame)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(data.m_DocumentID);
			if (!pResource) continue;
			UIDocumentData* pDocument = static_cast<UIDocumentData*>(pResource);

			UIDocument& document = GetDocument(data, pDocument);
			CalculateProjection(document.m_Projection, m_pEngine->ActiveGraphicsDevice(),
				0.0f, 0.0f, float(data.m_Resolution.x), float(data.m_Resolution.y));
			document.m_CursorPos = data.m_CursorPos;
			document.m_CursorScrollDelta = data.m_CursorScrollDelta;
			document.m_CursorDown = data.m_CursorDown;
			document.m_InputEnabled = data.m_InputEnabled;
			document.Update();
			document.Draw();
		}
	}

	void UIRendererModule::UIWorldSpaceQuadPass(uint32_t cameraIndex, RendererModule* pRenderer)
	{
		//CameraRef camera = pRenderer->GetActiveCamera(cameraIndex);
		//GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		//
		//for (auto& data : m_Frame)
		//{
		//	if (data.m_Target != UITarget::WorldSpaceQuad) continue;
		//
		//	/* Get document */
		//	auto& iter = m_Documents.find(data.m_ObjectID);
		//	if (iter == m_Documents.end()) continue;
		//	UIDocument& document = iter->second;
		//	RenderTexture* pDocumentTexture = document.m_pUITexture;
		//
		//	MaterialData* pMaterialData = m_pEngine->GetMaterialManager().GetMaterial(data.m_MaterialID);
		//	if (!pMaterialData) return;
		//	Material* pMaterial = pGraphics->UseMaterial(pMaterialData);
		//	if (!pMaterial) return;
		//
		//	ObjectData object;
		//	object.Model = data.m_WorldTransform;
		//	object.View = camera.GetView();
		//	object.Projection = camera.GetProjection();
		//	object.ObjectID = data.m_ObjectID;
		//	object.SceneID = data.m_SceneID;
		//
		//	MeshData* pMeshData = GetDocumentQuadMesh(data);
		//	pMaterial->SetSamplers(m_pEngine);
		//	pMaterial->ResetTextureCounter();
		//	pMaterial->SetTexture("texSampler", pDocumentTexture->GetTextureAttachment(0));
		//	pMaterial->SetPropertiesBuffer(m_pEngine);
		//	pMaterial->SetObjectData(object);
		//	pGraphics->DrawMesh(pMeshData, 0, pMeshData->VertexCount());
		//}
	}

	void UIRendererModule::UIOverlayPass(uint32_t cameraIndex, RendererModule* pRenderer)
	{
		//CameraRef camera = pRenderer->GetActiveCamera(cameraIndex);
		//GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		//GPUResourceManager* pResourceManager = pGraphics->GetResourceManager();
		//
		//RenderTexture* pCameraTexture = camera.GetOutputTexture();
		//RenderTexture* pOutputTexture = camera.GetSecondaryOutputTexture();
		//uint32_t width, height;
		//pOutputTexture->GetDimensions(width, height);
		//
		//pGraphics->EnableDepthTest(false);
		//pGraphics->SetViewport(0, 0, width, height);
		//
		//for (auto& data : m_Frame)
		//{
		//	if (data.m_Target != UITarget::CameraOverlay || camera.GetUUID() != data.m_TargetCamera) continue;
		//
		//	/* Get document */
		//	auto& iter = m_Documents.find(data.m_ObjectID);
		//	if (iter == m_Documents.end()) continue;
		//	UIDocument& document = iter->second;
		//	RenderTexture* pDocumentTexture = document.m_pUITexture;
		//
		//	pCameraTexture = camera.GetOutputTexture();
		//	pOutputTexture = camera.GetSecondaryOutputTexture();
		//
		//	/* Render to the output texture */
		//	pOutputTexture->BindForDraw();
		//
		//	/* Use overlay material */
		//	Material* pMaterial = pGraphics->UseMaterial(m_pUIOverlayMaterial);
		//
		//	/* Bind camera texture and document texture */
		//	pCameraTexture->BindAll(pMaterial);
		//	pDocumentTexture->BindAll(pMaterial);
		//
		//	/* Draw the screen quad */
		//	pGraphics->DrawScreenQuad();
		//
		//	pOutputTexture->UnBindForDraw();
		//
		//	pGraphics->UseMaterial(nullptr);
		//
		//	/* Swap the cameras textures for the next pass */
		//	camera.Swap();
		//}
		//
		///* Reset render textures and materials */
		//pGraphics->UseMaterial(nullptr);
		//pGraphics->EnableDepthTest(true);
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
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		RendererModule* pRenderer = m_pEngine->GetMainModule<RendererModule>();

		const UUID id = data.m_ObjectID;

		auto& iter = m_Documents.find(id);
		if (iter == m_Documents.end())
		{
			m_Documents.emplace(id, pDocument);
			UIDocument& newDocument = m_Documents.at(id);
			newDocument.CreateRenderPasses(pDevice, pRenderer->GetNumFramesInFlight(), glm::uvec2(uint32_t(data.m_Resolution.x), uint32_t(data.m_Resolution.y)));
			newDocument.m_SceneID = data.m_SceneID;
			newDocument.m_ObjectID = data.m_ObjectID;
			newDocument.m_pRenderer = this;

			return newDocument;
		}

		UIDocument& document = iter->second;
		uint32_t width, height;
		document.GetResolution(width, height);
		if (width != data.m_Resolution.x || height != data.m_Resolution.y)
		{
			document.ResizeRenderTexture(pDevice, pRenderer->GetNumFramesInFlight(),
				glm::uvec2(uint32_t(data.m_Resolution.x), uint32_t(data.m_Resolution.y)));
			document.m_DrawIsDirty = true;
		}

		if (document.m_OriginalDocumentID != pDocument->GetUUID() || forceCreate)
		{
			m_Documents.erase(iter);
			m_Documents.emplace(id, pDocument);
			UIDocument& newDocument = m_Documents.at(id);
			newDocument.m_SceneID = data.m_SceneID;
			newDocument.m_ObjectID = data.m_ObjectID;
			newDocument.m_UIPasses = std::move(document.m_UIPasses);
			newDocument.m_Resolution = document.m_Resolution;
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
