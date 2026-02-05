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
#include <InternalTexture.h>
#include <FontDataStructs.h>
#include <MaterialData.h>
#include <SceneManager.h>
#include <LocalizeModuleBase.h>

#include <DistributedRandom.h>

#include <EntityRegistry.h>
#include <GloryECS/ComponentTypes.h>
#include <Reflection.h>
#include <PropertyFlags.h>
#include <VertexHelpers.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <Renderer.h>

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
		m_Documents.clear();
		m_Frame.clear();
		m_pDocumentQuads.clear();
		m_BatchDatas.clear();
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

		AssetManager& assets = m_pEngine->GetAssetManager();

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

		for (size_t i = 0; i < pDocument->m_UIBatch.m_Worlds.size(); ++i)
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
		pDevice->SetRenderPassClear(renderPass, data.m_ClearColor);
		pDevice->BeginRenderPass(commandBuffer, renderPass);

		uint8_t mask = 0;
		UIConstants constants;
		constants.Projection = pDocument->m_Projection;
		for (size_t i = 0; i < pDocument->m_UIBatch.m_Worlds.size(); ++i)
		{
			/* Setup constants */
			constants.ObjectIndex = i;
			constants.ColorIndex = pDocument->m_UIBatch.m_ColorIndices[i];
			constants.HasTexture = pDocument->m_UIBatch.m_TextureIDs[i] ? 1 : 0;

			if (pDocument->m_UIBatch.m_MaskDecrements.IsSet(i))
			{
				/* Remove the shape from the stencil */
				--mask;
				pDevice->BeginPipeline(commandBuffer, m_UIStencilPipeline);
				pDevice->SetStencilOp(commandBuffer, CompareOp::OP_Equal, Func::OP_Keep, Func::OP_Keep, Func::OP_Decrement, mask + 1, 255);
				pDevice->PushConstants(commandBuffer, m_UIStencilPipeline, 0, sizeof(UIConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
				pDevice->BindDescriptorSets(commandBuffer, m_UIStencilPipeline, { batchData.m_BuffersSet });
				pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(pDocument->m_Resolution.x), float(pDocument->m_Resolution.y));
				pDevice->SetScissor(commandBuffer, 0, 0, pDocument->m_Resolution.x, pDocument->m_Resolution.y);
				pDevice->DrawMesh(commandBuffer, m_ImageMesh);
				pDevice->EndPipeline(commandBuffer);
				continue;
			}

			const bool isStencil = pDocument->m_UIBatch.m_MaskIncrements.IsSet(i);
			if (isStencil)
			{
				/* Render the mask to the stencil buffer */
				pDevice->BeginPipeline(commandBuffer, m_UIStencilPipeline);
				/* First stencil pass increases stencil value by 1 */
				pDevice->SetStencilOp(commandBuffer, CompareOp::OP_Always, Func::OP_Keep, Func::OP_Keep, Func::OP_Increment, mask, 255);
				pDevice->PushConstants(commandBuffer, m_UIStencilPipeline, 0, sizeof(UIConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
				pDevice->BindDescriptorSets(commandBuffer, m_UIStencilPipeline, { batchData.m_BuffersSet });
				pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(pDocument->m_Resolution.x), float(pDocument->m_Resolution.y));
				pDevice->SetScissor(commandBuffer, 0, 0, pDocument->m_Resolution.x, pDocument->m_Resolution.y);
				pDevice->DrawMesh(commandBuffer, m_ImageMesh);
				/* Second stencil pass compares stencil with the the expected addition, on fail it is reduced by 1 */
				pDevice->SetStencilOp(commandBuffer, CompareOp::OP_Equal, Func::OP_Decrement, Func::OP_Decrement, Func::OP_Replace, mask + 1, 255);
				pDevice->DrawMesh(commandBuffer, m_ImageMesh);
				pDevice->EndPipeline(commandBuffer);

				++mask;
				continue;
			}

			const UUID meshID = pDocument->m_UIBatch.m_TextMeshes[i];

			MeshData* pMesh = meshID ? pDocument->m_pTextMeshes.at(meshID).get() : nullptr;
			MeshHandle mesh = pMesh ? pDevice->AcquireCachedMesh(pMesh, MU_Dynamic) : m_ImageMesh;

			const PipelineHandle pipeline = meshID ? m_UITextPipeline : m_UIPipeline;
			pDevice->BeginPipeline(commandBuffer, pipeline);

			if (mask > 0)
			{
				pDevice->SetStencilTestEnabled(commandBuffer, true);
				pDevice->SetStencilOp(commandBuffer, CompareOp::OP_LessOrEqual, Func::OP_Keep, Func::OP_Keep, Func::OP_Keep, mask, 255);
			}

			pDevice->PushConstants(commandBuffer, pipeline, 0, sizeof(UIConstants), &constants, ShaderTypeFlag(STF_Vertex | STF_Fragment));
			pDevice->BindDescriptorSets(commandBuffer, pipeline, { batchData.m_BuffersSet, batchData.m_TextureSets[i] });
			pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(pDocument->m_Resolution.x), float(pDocument->m_Resolution.y));
			pDevice->SetScissor(commandBuffer, 0, 0, pDocument->m_Resolution.x, pDocument->m_Resolution.y);
			pDevice->DrawMesh(commandBuffer, mesh);
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

	const DescriptorSetLayoutHandle& UIRendererModule::UIOverlaySetLayout() const
	{
		return m_UIOverlaySamplerLayout;
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

		Renderer* pRenderer = m_pEngine->ActiveRenderer();
		pRenderer->InjectPreRenderPass([this](GraphicsDevice* pDevice, CommandBufferHandle commandBuffer, uint32_t frameIndex) {
			UIPrepass(pDevice, commandBuffer, frameIndex);
		});

		PostProcess uiOverlayPostProcess;
		uiOverlayPostProcess.m_Priority = INT32_MIN;
		uiOverlayPostProcess.m_Name = "UI Overlay";
		uiOverlayPostProcess.m_Callback =
		[this](GraphicsDevice* pDevice, CameraRef camera, size_t, CommandBufferHandle commandBuffer,
			size_t frameIndex, RenderPassHandle renderPass, DescriptorSetHandle ds)
		{
			return UIOverlayPass(pDevice, camera, commandBuffer, frameIndex, renderPass, ds);
		};
		pRenderer->AddPostProcess(std::move(uiOverlayPostProcess));

		pRenderer->InjectDatapass([this](GraphicsDevice* pDevice, Renderer* pRenderer)
			{ UIDataPass(pDevice, pRenderer); });
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

		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();

		DescriptorSetLayoutInfo samplerSetLayoutInfo;
		samplerSetLayoutInfo.m_SamplerNames = { "Color" };
		samplerSetLayoutInfo.m_Samplers.resize(1);
		samplerSetLayoutInfo.m_Samplers[0].m_BindingIndex = 0;
		samplerSetLayoutInfo.m_Samplers[0].m_ShaderStages = STF_Fragment;
		m_UISamplerLayout = pDevice->CreateDescriptorSetLayout(std::move(samplerSetLayoutInfo));

		samplerSetLayoutInfo = DescriptorSetLayoutInfo();
		samplerSetLayoutInfo.m_SamplerNames = { "UIColor" };
		samplerSetLayoutInfo.m_Samplers.resize(1);
		samplerSetLayoutInfo.m_Samplers[0].m_BindingIndex = 1;
		samplerSetLayoutInfo.m_Samplers[0].m_ShaderStages = STF_Fragment;
		m_UIOverlaySamplerLayout = pDevice->CreateDescriptorSetLayout(std::move(samplerSetLayoutInfo));

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

		RenderPassInfo renderPassInfo;
		renderPassInfo.RenderTextureInfo.HasDepth = false;
		renderPassInfo.RenderTextureInfo.HasStencil = true;
		renderPassInfo.RenderTextureInfo.Width = 1;
		renderPassInfo.RenderTextureInfo.Height = 1;
		renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("UIColor", PixelFormat::PF_RGBA,
			PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));
		renderPassInfo.m_CreateRenderTexture = true;

		m_DummyRenderPass = pDevice->CreateRenderPass(std::move(renderPassInfo));
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

	void UIRendererModule::Load()
	{
		Renderer* pRenderer = m_pEngine->ActiveRenderer();
		if (!pRenderer) return;
		GraphicsDevice* pDevice = m_pEngine->ActiveGraphicsDevice();
		if (!pDevice) return;

		CheckCachedPipelines(pDevice);

		RenderPassHandle renderPass = pRenderer->GetDummyPostProcessPass();
		if (!renderPass) return;

		CheckCachedOverlayPipeline(renderPass, pDevice);
	}

	void UIRendererModule::UIPrepass(GraphicsDevice* pDevice, CommandBufferHandle commandBuffer, uint32_t frameIndex)
	{
		for (auto& data : m_Frame)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(data.m_DocumentID);
			if (!pResource) continue;
			UIDocumentData* pDocument = static_cast<UIDocumentData*>(pResource);

			UIDocument& document = GetDocument(data, pDocument);
			document.m_CursorPos = data.m_CursorPos;
			document.m_CursorScrollDelta = data.m_CursorScrollDelta;
			document.m_CursorDown = data.m_CursorDown;
			document.m_InputEnabled = data.m_InputEnabled;

			DrawDocument(pDevice, commandBuffer, frameIndex, &document, data);
		}
	}

	void UIRendererModule::UIDataPass(GraphicsDevice* pDevice, Renderer* pRenderer)
	{
		CheckCachedPipelines(pDevice);

		MaterialManager& materials = m_pEngine->GetMaterialManager();
		AssetManager& assets = m_pEngine->GetAssetManager();

		for (size_t i = 0; i < m_Frame.size(); ++i)
		{
			const auto& data = m_Frame[i];
			if (data.m_Target != UITarget::WorldSpaceQuad) continue;

			/* Get document */
			auto& iter = m_Documents.find(data.m_ObjectID);
			if (iter == m_Documents.end()) continue;
			UIDocument& document = iter->second;

			/* Update material */
			MaterialData* pMaterial = materials.GetMaterial(data.m_MaterialID);
			if (!pMaterial) continue;

			TextureData* pOldTexture = nullptr;
			if (!pMaterial->GetTexture(data.m_MaterialTextureName, &pOldTexture, &assets))
				continue;

			auto textures = GetDocumentTexture(pDevice, data, document, pRenderer->GetNumFramesInFlight());
			TextureData* pTexture = textures[pRenderer->GetCurrentFrameInFlight()];
			if (pOldTexture != pTexture)
			{
				pMaterial->SetTexture(data.m_MaterialTextureName, pTexture);
				pMaterial->SetDirty(true);
			}
			if (pTexture->IsDirty())
			{
				pTexture->SetDirty(false);
				pMaterial->SetDirty(true);
			}

			/* Submit data to renderer */
			MeshData* pMesh = GetDocumentQuadMesh(data);
			RenderData renderData;
			renderData.m_DepthWrite = true;
			renderData.m_LayerMask = data.m_LayerMask;
			renderData.m_MeshID = pMesh->GetUUID();
			renderData.m_ObjectID = data.m_ObjectID;
			renderData.m_SceneID = data.m_SceneID;
			renderData.m_World = data.m_WorldTransform;
			renderData.m_MaterialID = data.m_MaterialID;
			pRenderer->SubmitDynamic(std::move(renderData));
		}
	}

	bool UIRendererModule::UIOverlayPass(GraphicsDevice* pDevice, CameraRef camera,
		CommandBufferHandle commandBuffer, size_t frameIndex, RenderPassHandle renderPass, DescriptorSetHandle ds)
	{
		CheckCachedOverlayPipeline(renderPass, pDevice);

		const glm::uvec2& resolution = camera.GetResolution();
		
		bool rendered = false;

		for (auto& data : m_Frame)
		{
			if (data.m_Target != UITarget::CameraOverlay || camera.GetUUID() != data.m_TargetCamera) continue;
		
			/* Get document */
			auto& iter = m_Documents.find(data.m_ObjectID);
			if (iter == m_Documents.end()) continue;
			UIDocument& document = iter->second;

			pDevice->BeginRenderPass(commandBuffer, renderPass);
			pDevice->BeginPipeline(commandBuffer, m_UIOverlayPipeline);
			pDevice->SetViewport(commandBuffer, 0.0f, 0.0f, float(resolution.x), float(resolution.y));
			pDevice->SetScissor(commandBuffer, 0, 0, resolution.x, resolution.y);
			pDevice->BindDescriptorSets(commandBuffer, m_UIOverlayPipeline, { ds, document.m_UIOverlaySets[frameIndex] });
			pDevice->DrawQuad(commandBuffer);
			pDevice->EndPipeline(commandBuffer);
			pDevice->EndRenderPass(commandBuffer);

			rendered = true;
		}
		return rendered;
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
		Renderer* pRenderer = m_pEngine->ActiveRenderer();

		const UUID id = data.m_ObjectID;

		auto& iter = m_Documents.find(id);
		if (iter == m_Documents.end())
		{
			m_Documents.emplace(id, pDocument);
			UIDocument& newDocument = m_Documents.at(id);
			newDocument.CreateRenderPasses(pDevice, pRenderer->GetNumFramesInFlight(), glm::uvec2(uint32_t(data.m_Resolution.x), uint32_t(data.m_Resolution.y)), this);
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
				glm::uvec2(uint32_t(data.m_Resolution.x), uint32_t(data.m_Resolution.y)), this);
			document.m_DrawIsDirty = true;
		}

		if (document.m_OriginalDocumentID != pDocument->GetUUID() || forceCreate)
		{
			std::vector<RenderPassHandle> passes = std::move(document.m_UIPasses);
			std::vector<DescriptorSetHandle> overlaySets = std::move(document.m_UIOverlaySets);
			const glm::uvec2 resolution = document.m_Resolution;
			m_Documents.erase(iter);
			m_Documents.emplace(id, pDocument);
			UIDocument& newDocument = m_Documents.at(id);
			newDocument.m_SceneID = data.m_SceneID;
			newDocument.m_ObjectID = data.m_ObjectID;
			if (!passes.empty())
				newDocument.m_UIPasses = std::move(passes);
			if (!overlaySets.empty())
				newDocument.m_UIOverlaySets = std::move(overlaySets);
			newDocument.m_Resolution = resolution;
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

			m_pEngine->GetAssetManager().AddLoadedResource(iter->second);

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

		return iter->second;
	}

	std::vector<TextureData*>& UIRendererModule::GetDocumentTexture(GraphicsDevice* pDevice,
		const UIRenderData& data, UIDocument& document, size_t imageCount)
	{
		auto iter = m_pDocumentTextures.find(data.m_ObjectID);
		if (iter == m_pDocumentTextures.end())
		{
			iter = m_pDocumentTextures.emplace(data.m_ObjectID, std::vector<TextureData*>(imageCount, nullptr)).first;
			for (size_t i = 0; i < imageCount; ++i)
			{
				ImageData* pImage = new ImageData();
				pImage->SetDirty(false);
				iter->second[i] = new InternalTexture(pImage);
				iter->second[i]->GetSamplerSettings().MipmapMode = Filter::F_None;
				m_pEngine->GetAssetManager().AddLoadedResource(iter->second[i]);
			}
		}

		iter->second.resize(imageCount);
		for (size_t i = 0; i < imageCount; ++i)
		{
			if (!iter->second[i])
			{
				ImageData* pImage = new ImageData();
				pImage->SetDirty(false);
				iter->second[i] = new InternalTexture(pImage);
				iter->second[i]->GetSamplerSettings().MipmapMode = Filter::F_None;
				m_pEngine->GetAssetManager().AddLoadedResource(iter->second[i]);
			}
			TextureHandle texture = pDevice->GetCachedTexture(iter->second[i]);
			RenderPassHandle renderPass = document.m_UIPasses[i];
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(renderPass);
			TextureHandle currentTexture = pDevice->GetRenderTextureAttachment(renderTexture, 0);
			if (texture != currentTexture)
			{
				pDevice->SetCachedTexture(iter->second[i], currentTexture);
				iter->second[i]->SetDirty(true);
			}
		}

		return iter->second;
	}

	void UIRendererModule::CheckCachedPipelines(GraphicsDevice* pDevice)
	{
		/* Prepare pipelines */
		const ModuleSettings& settings = Settings();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		PipelineData* pPipeline = pipelines.GetPipelineData(settings.Value<uint64_t>("UI Prepass Pipeline"));
		m_UIPipeline = pDevice->AcquireCachedPipeline(m_DummyRenderPass, pPipeline,
			{ m_UIBuffersLayout, m_UISamplerLayout }, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
		pPipeline = pipelines.GetPipelineData(settings.Value<uint64_t>("UI Text Prepass Pipeline"));
		m_UITextPipeline = pDevice->AcquireCachedPipeline(m_DummyRenderPass, pPipeline,
			{ m_UIBuffersLayout, m_UISamplerLayout }, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
		pPipeline = pipelines.GetPipelineData(settings.Value<uint64_t>("UI Prepass Stencil Pipeline"));
		m_UIStencilPipeline = pDevice->AcquireCachedPipeline(m_DummyRenderPass, pPipeline,
			{ m_UIBuffersLayout }, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
	}

	void UIRendererModule::CheckCachedOverlayPipeline(RenderPassHandle renderPass, GraphicsDevice* pDevice)
	{
		const ModuleSettings& settings = Settings();
		PipelineManager& pipelines = m_pEngine->GetPipelineManager();
		PipelineData* pPipeline = pipelines.GetPipelineData(settings.Value<uint64_t>("UI Overlay Pipeline"));
		m_UIOverlayPipeline = pDevice->AcquireCachedPipeline(renderPass, pPipeline,
			{ m_UISamplerLayout, m_UIOverlaySamplerLayout }, sizeof(glm::vec3), { AttributeType::Float3 });
	}
}
