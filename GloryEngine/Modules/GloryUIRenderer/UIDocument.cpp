#include "UIDocument.h"
#include "UIDocumentData.h"
#include "UIComponents.h"
#include "UIRendererModule.h"

#include <FontData.h>
#include <MeshData.h>
#include <RenderData.h>
#include <GraphicsDevice.h>
#include <VertexHelpers.h>

#include <queue>

namespace Glory
{
	void UIDocument::CopyEntity(Utils::ECS::EntityRegistry& registry, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent)
	{
		const Utils::ECS::EntityID newEntity = registry.CopyEntityToOtherRegistry(entity, parent, &m_Registry);
		Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
		{
			const Utils::ECS::EntityID child = pEntityView->Child(i);
			CopyEntity(registry, child, newEntity);
		}
	}

	UUID UIDocument::CopyEntity(UIDocumentData* pOtherDocument, Utils::ECS::EntityID entity, Utils::ECS::EntityID parent)
	{
		Utils::ECS::EntityRegistry& registry = pOtherDocument->GetRegistry();
		const Utils::ECS::EntityID newEntity = registry.CopyEntityToOtherRegistry(entity, parent, &m_Registry);
		const UUID uuid = UUID();
		m_Ids.emplace(uuid, newEntity);
		m_UUIds.emplace(newEntity, uuid);
		m_Names.emplace(newEntity, pOtherDocument->Name(entity));

		Utils::ECS::EntityView* pEntityView = registry.GetEntityView(entity);
		for (size_t i = 0; i < pEntityView->ChildCount(); ++i)
		{
			const Utils::ECS::EntityID child = pEntityView->Child(i);
			CopyEntity(pOtherDocument, child, newEntity);
		}
		return uuid;
	}

	void UIDocument::UpdateEntityActiveHierarchy(Utils::ECS::EntityID entity)
	{
		const Utils::ECS::EntityID parent = m_Registry.GetParent(entity);
		Utils::ECS::EntityView* pEntity = m_Registry.GetEntityView(entity);
		Utils::ECS::EntityView* pParent = parent ? m_Registry.GetEntityView(parent) : nullptr;

		const bool activeSelf = pEntity->Active();
		const bool activeHierarchy = activeSelf && (pParent ? pParent->HierarchyActive() : true);
		pEntity->HierarchyActive() = activeSelf && activeHierarchy;
		m_Registry.SetEntityDirty(entity);

		for (size_t i = 0; i < m_Registry.ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(entity, i);
			UpdateEntityActiveHierarchy(child);
		}
	}

	UIDocument::UIDocument(UIDocumentData* pDocument):
		m_OriginalDocumentID(pDocument->GetUUID()), m_Resolution(1, 1), m_SceneID(0), m_ObjectID(0),
		m_pRenderer(nullptr), m_Projection(glm::identity<glm::mat4>()),
		m_CursorPos(0.0f, 0.0f), m_CursorScrollDelta(0.0f, 0.0f), m_CursorDown(false),
		m_WasCursorDown(false), m_InputEnabled(true), m_PanelCounter(0), m_Name(pDocument->m_Name),
		m_Ids(pDocument->m_Ids), m_UUIds(pDocument->m_UUIds), m_Names(pDocument->m_Names), m_DrawIsDirty(32, true)
	{
		Utils::ECS::EntityRegistry& registry = pDocument->GetRegistry();
		for (size_t i = 0; i < registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = registry.Child(0, i);
			CopyEntity(registry, child, 0);
		}
	}

	static void UpdateEntity(Utils::ECS::EntityID entity, Utils::ECS::EntityRegistry& registry, Utils::ECS::InvocationType invocation)
	{
		registry.InvokeAll(invocation, { entity });
		for (size_t i = 0; i < registry.ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = registry.Child(entity, i);
			UpdateEntity(child, registry, invocation);
		}
	}

	void UIDocument::Update()
	{
		m_Registry.SetUserData(this);

		/* Update all transforms first so we have a base for other components */
		m_Registry.GetTypeView<UITransform>()->InvokeAll(Utils::ECS::InvocationType::PostUpdate, &m_Registry, NULL);
		for (size_t i = 0; i < m_Registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(0, i);
			UpdateEntity(child, m_Registry, Utils::ECS::InvocationType::PreUpdate);
		}
		for (size_t i = 0; i < m_Registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(0, i);
			UpdateEntity(child, m_Registry, Utils::ECS::InvocationType::Update);
		}
		for (size_t i = 0; i < m_Registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(0, i);
			UpdateEntity(child, m_Registry, Utils::ECS::InvocationType::PostUpdate);
		}
		m_WasCursorDown = m_CursorDown;
	}

	void DrawEntity(Utils::ECS::EntityID entity, Utils::ECS::EntityRegistry& registry)
	{
		registry.InvokeAll(Utils::ECS::InvocationType::Draw, { entity });
		for (size_t i = 0; i < registry.ChildCount(entity); ++i)
		{
			const Utils::ECS::EntityID child = registry.Child(entity, i);
			DrawEntity(child, registry);
		}
		registry.InvokeAll(Utils::ECS::InvocationType::PostDraw, { entity });
	}

	void UIDocument::Draw()
	{
		m_UIBatch.m_MaskIncrements.Clear();
		m_UIBatch.m_MaskDecrements.Clear();

		m_PanelCounter = 0;
		m_Registry.SetUserData(this);

		m_UIBatch.Reset();

		for (size_t i = 0; i < m_Registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(0, i);
			DrawEntity(child, m_Registry);
		}
	}

	UUID UIDocument::OriginalDocumentID() const
	{
		return m_OriginalDocumentID;
	}

	UUID UIDocument::SceneID() const
	{
		return m_SceneID;
	}

	UUID UIDocument::ObjectID() const
	{
		return m_ObjectID;
	}

	UIRendererModule* UIDocument::Renderer()
	{
		return m_pRenderer;
	}

	glm::mat4& UIDocument::Projection()
	{
		return m_Projection;
	}

	const glm::mat4& UIDocument::Projection() const
	{
		return m_Projection;
	}

	UUID UIDocument::GetTextMesh(UUID objectID, const TextData& data, FontData* pFont)
	{
		auto iter = m_pTextMeshes.find(objectID);
		const bool exists = iter != m_pTextMeshes.end();
		if (!exists)
		{
			MeshData* pMesh = new MeshData(data.m_Text.size() * 4, sizeof(VertexPosColorTex),
				{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
			iter = m_pTextMeshes.emplace(objectID, pMesh).first;
		}

		if (data.m_TextDirty || !exists)
			Utils::GenerateTextMesh(iter->second.get(), pFont, data);
		return iter->first;
	}

	const glm::uvec2& UIDocument::GetResolution() const
	{
		return m_Resolution;
	}

	void UIDocument::GetResolution(uint32_t& width, uint32_t& height) const
	{
		width = m_Resolution.x;
		height = m_Resolution.y;
	}

	Utils::ECS::EntityRegistry& UIDocument::Registry()
	{
		return m_Registry;
	}

	std::string_view UIDocument::Name() const
	{
		return m_Name;
	}

	std::string_view UIDocument::Name(Utils::ECS::EntityID entity) const
	{
		return m_Names.at(entity);
	}

	void UIDocument::SetName(Utils::ECS::EntityID entity, std::string_view name)
	{
		auto& iter = m_Names.find(entity);
		if (iter == m_Names.end()) return;
		iter->second = name;
	}

	UUID UIDocument::EntityUUID(Utils::ECS::EntityID entity) const
	{
		return m_UUIds.at(entity);
	}

	Utils::ECS::EntityID UIDocument::EntityID(UUID uuid) const
	{
		return m_Ids.at(uuid);
	}

	Utils::ECS::EntityID UIDocument::CreateEmptyEntity(std::string_view name, UUID uuid)
	{
		Utils::ECS::EntityID entity = m_Registry.CreateEntity();
		m_UUIds.emplace(entity, uuid);
		m_Ids.emplace(uuid, entity);
		m_Names.emplace(entity, name);
		return entity;
	}

	Utils::ECS::EntityID UIDocument::CreateEntity(std::string_view name, UUID uuid)
	{
		Utils::ECS::EntityID entity = m_Registry.CreateEntity<UITransform>();
		m_UUIds.emplace(entity, uuid);
		m_Ids.emplace(uuid, entity);
		m_Names.emplace(entity, name);
		return entity;
	}

	void UIDocument::DestroyEntity(UUID uuid)
	{
		const Utils::ECS::EntityID entity = m_Ids.at(uuid);
		while (m_Registry.ChildCount(entity) > 0)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(entity, 0);
			const UUID uuid = m_UUIds.at(child);
			DestroyEntity(uuid);
		}

		m_Registry.DestroyEntity(entity);
		m_Ids.erase(uuid);
		m_UUIds.erase(entity);
		m_Names.erase(entity);
	}

	bool UIDocument::EntityExists(UUID uuid)
	{
		return m_Ids.find(uuid) != m_Ids.end();
	}

	size_t UIDocument::ElementCount()
	{
		return m_Ids.size();
	}

	UUID UIDocument::FindElement(UUID parentId, std::string_view name)
	{
		const Utils::ECS::EntityID parent = parentId ? m_Ids.at(parentId) : 0;

		for (size_t i = 0; i < m_Registry.ChildCount(parent); i++)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(parent, i);
			const std::string_view otherName = m_Names.at(child);
			if (name != otherName) continue;
			return m_UUIds.at(child);
		}
		return 0;
	}

	const glm::vec2& UIDocument::GetCursorPos() const
	{
		return m_CursorPos;
	}

	const glm::vec2& UIDocument::GetCursorScrollDelta() const
	{
		return m_CursorScrollDelta;
	}

	bool UIDocument::IsCursorDown() const
	{
		return m_CursorDown;
	}

	bool UIDocument::WasCursorDown() const
	{
		return m_WasCursorDown;
	}

	bool UIDocument::IsEnputEnabled() const
	{
		return m_InputEnabled;
	}

	void UIDocument::SetEnputEnabled(bool enabled)
	{
		m_InputEnabled = enabled;
	}

	size_t& UIDocument::PanelCounter()
	{
		return m_PanelCounter;
	}

	void UIDocument::SetDrawDirty()
	{
		m_DrawIsDirty.SetAll();
	}

	UUID UIDocument::Instantiate(UIDocumentData* pOtherDocument, UUID parentID)
	{
		UUID firstElementID = 0;
		const Utils::ECS::EntityID parent = parentID ? m_Ids.at(parentID) : 0;
		Utils::ECS::EntityRegistry& registry = pOtherDocument->GetRegistry();
		for (size_t i = 0; i < registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = registry.Child(0, i);
			const UUID uuid = CopyEntity(pOtherDocument, child, parent);
			if (i == 0) firstElementID = uuid;
		}
		return firstElementID;
	}

	void UIDocument::SetAllEntitiesDirty()
	{
		for (size_t i = 0; i < m_Registry.ChildCount(0); ++i)
		{
			const Utils::ECS::EntityID child = m_Registry.Child(0, i);
			m_Registry.SetEntityDirty(child);
		}
	}

	void UIDocument::SetEntityActive(Utils::ECS::EntityID entity, bool active)
	{
		m_Registry.GetEntityView(entity)->Active() = active;
		UpdateEntityActiveHierarchy(entity);
		m_DrawIsDirty.SetAll();
	}

	void UIDocument::Start()
	{
		m_Registry.SetUserData(this);
		m_Registry.InvokeAll(Utils::ECS::InvocationType::Start, NULL);
	}

	void UIDocument::SetEntityDirty(Utils::ECS::EntityID entity, bool setChildrenDirty, bool setParentsDirty)
	{
		m_Registry.SetEntityDirty(entity, true, setChildrenDirty);
		Utils::ECS::EntityID parent = m_Registry.GetParent(entity);
		while (setParentsDirty && parent)
		{
			m_Registry.SetEntityDirty(parent, true, false);
			parent = m_Registry.GetParent(parent);
		}
	}

	void UIDocument::AddRender(UUID textMeshID, UUID textureID, glm::mat4&& world, const glm::vec4& color)
	{
		const size_t index = m_UIBatch.m_Worlds.size();
		m_UIBatch.m_TextMeshes.emplace_back(textMeshID);
		m_UIBatch.m_Worlds.emplace_back(std::move(world));
		m_UIBatch.m_TextureIDs.emplace_back(textureID);

		auto iter = std::find(m_UIBatch.m_UniqueColors.begin(), m_UIBatch.m_UniqueColors.end(), color);

		if (iter == m_UIBatch.m_UniqueColors.end())
		{
			m_UIBatch.m_ColorIndices.emplace_back(static_cast<uint32_t>(m_UIBatch.m_UniqueColors.size()));
			m_UIBatch.m_UniqueColors.emplace_back(color);
			return;
		}

		const uint32_t materialIndex = static_cast<uint32_t>(iter - m_UIBatch.m_UniqueColors.begin());
		m_UIBatch.m_ColorIndices.emplace_back(materialIndex);

		m_UIBatch.m_MaskIncrements.Reserve(m_UIBatch.m_Worlds.size());
		m_UIBatch.m_MaskDecrements.Reserve(m_UIBatch.m_Worlds.size());
		m_UIBatch.m_MaskIncrements.Set(index, false);
		m_UIBatch.m_MaskDecrements.Set(index, false);
	}

	void UIDocument::BeginMask(glm::mat4&& world)
	{
		const size_t index = m_UIBatch.m_Worlds.size();
		m_UIBatch.m_TextMeshes.emplace_back(0);
		m_UIBatch.m_Worlds.emplace_back(std::move(world));
		m_UIBatch.m_TextureIDs.emplace_back(0);
		m_UIBatch.m_ColorIndices.emplace_back(0);
		m_UIBatch.m_MaskIncrements.Reserve(m_UIBatch.m_Worlds.size());
		m_UIBatch.m_MaskDecrements.Reserve(m_UIBatch.m_Worlds.size());
		m_UIBatch.m_MaskIncrements.Set(index, true);
		m_UIBatch.m_MaskDecrements.Set(index, false);
	}

	void UIDocument::EndMask()
	{
		const size_t index = m_UIBatch.m_Worlds.size();
		m_UIBatch.m_TextMeshes.emplace_back(0);
		m_UIBatch.m_Worlds.emplace_back(glm::identity<glm::mat4>());
		m_UIBatch.m_TextureIDs.emplace_back(0);
		m_UIBatch.m_ColorIndices.emplace_back(0);
		m_UIBatch.m_MaskIncrements.Reserve(m_UIBatch.m_Worlds.size());
		m_UIBatch.m_MaskDecrements.Reserve(m_UIBatch.m_Worlds.size());
		m_UIBatch.m_MaskIncrements.Set(index, false);
		m_UIBatch.m_MaskDecrements.Set(index, true);
	}

	void UIDocument::CreateRenderPasses(GraphicsDevice* pDevice, size_t imageCount, const glm::uvec2& resolution, UIRendererModule* pUIRenderer)
	{
		m_Resolution = resolution;

		m_UIPasses.resize(imageCount, nullptr);
		m_UIOverlaySets.resize(imageCount, nullptr);
		for (size_t i = 0; i < m_UIPasses.size(); ++i)
		{
			if (m_UIPasses[i]) continue;

			RenderPassInfo renderPassInfo;
			renderPassInfo.RenderTextureInfo.HasDepth = false;
			renderPassInfo.RenderTextureInfo.HasStencil = true;
			renderPassInfo.RenderTextureInfo.Width = m_Resolution.x;
			renderPassInfo.RenderTextureInfo.Height = m_Resolution.y;
			renderPassInfo.RenderTextureInfo.Attachments.push_back(Attachment("UIColor", PixelFormat::PF_RGBA,
				PixelFormat::PF_R8G8B8A8Srgb, Glory::ImageType::IT_2D, Glory::ImageAspect::IA_Color, DataType::DT_Float));

			m_UIPasses[i] = pDevice->CreateRenderPass(std::move(renderPassInfo));
			RenderTextureHandle uiRenderTexture = pDevice->GetRenderPassRenderTexture(m_UIPasses[i]);
			TextureHandle uiTexture = pDevice->GetRenderTextureAttachment(uiRenderTexture, 0);

			DescriptorSetInfo dsInfo;
			dsInfo.m_Layout = pUIRenderer->UIOverlaySetLayout();
			dsInfo.m_Samplers.resize(1);
			dsInfo.m_Samplers[0].m_TextureHandle = uiTexture;
			m_UIOverlaySets[i] = pDevice->CreateDescriptorSet(std::move(dsInfo));
		}
		m_DrawIsDirty.SetAll();
	}

	void UIDocument::ResizeRenderTexture(GraphicsDevice* pDevice, size_t imageCount, const glm::uvec2& resolution, UIRendererModule* pUIRenderer)
	{
		if (m_UIPasses.size() < imageCount)
			CreateRenderPasses(pDevice, imageCount, resolution, pUIRenderer);

		pDevice->WaitIdle();
		
		for (size_t i = 0; i < m_UIPasses.size(); ++i)
		{
			RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_UIPasses[i]);
			pDevice->ResizeRenderTexture(renderTexture, resolution.x, resolution.y);
			TextureHandle uiTexture = pDevice->GetRenderTextureAttachment(renderTexture, 0);

			DescriptorSetUpdateInfo dsWriteInfo;
			dsWriteInfo.m_Samplers.resize(1);
			dsWriteInfo.m_Samplers[0].m_TextureHandle = uiTexture;
			dsWriteInfo.m_Samplers[0].m_DescriptorIndex = 0;
			pDevice->UpdateDescriptorSet(m_UIOverlaySets[i], dsWriteInfo);
		}
		m_DrawIsDirty.SetAll();
	}

	void UIDocument::SetClearColor(const glm::vec4& color)
	{
		m_ClearColor = color;
	}

	TextureHandle UIDocument::GetUITexture(GraphicsDevice* pDevice, uint32_t frameIndex)
	{
		RenderTextureHandle renderTexture = pDevice->GetRenderPassRenderTexture(m_UIPasses[frameIndex]);
		return pDevice->GetRenderTextureAttachment(renderTexture, 0);
	}
}
