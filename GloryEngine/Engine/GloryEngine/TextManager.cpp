#include "TextManager.h"

#include "GScene.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "IEngine.h"
#include "VertexHelpers.h"
#include "RenderData.h"
#include "Resources.h"

#include <EntityRegistry.h>

namespace Glory
{
	TextManager::TextManager(Utils::ECS::EntityRegistry* pRegistry, size_t capacity):
		ComponentManager(pRegistry, capacity), m_pSceneManager(nullptr),
		m_pResources(nullptr), m_pLayerManager(nullptr)
	{
	}

	TextManager::~TextManager()
	{
	}

	void TextManager::OnDrawImpl(Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
		if (!pComponent.m_Font || pComponent.m_Text.empty()) return;

		GScene* pScene = m_pRegistry->GetUserData<GScene>();
		Renderer* pRenderer = m_pSceneManager->GetRenderer();
		if (!pRenderer) return;

		FontData* pFont = pComponent.m_Font.Get(m_pResources);
		if (!pFont) return;

		Transform& transform = m_pRegistry->GetComponent<Transform>(entity);

		LayerMask mask;
		if (m_pRegistry->HasComponent<LayerComponent>(entity))
		{
			LayerComponent& layer = m_pRegistry->GetComponent<LayerComponent>(entity);
			mask = layer.m_Layer.Layer(m_pLayerManager) != nullptr ? layer.m_Layer.Layer(m_pLayerManager)->m_Mask : LayerMask(0ull);
		}

		const UUID fontID = pComponent.m_Font.GetUUID();

		TextData textData;
		textData.m_Text = pComponent.m_Text;
		textData.m_TextDirty = pComponent.m_Dirty;
		textData.m_Scale = pComponent.m_Scale;
		textData.m_Alignment = pComponent.m_Alignment;
		textData.m_TextWrap = pComponent.m_WrapWidth;
		textData.m_Color = pComponent.m_Color;
		pComponent.m_Dirty = false;

		RenderData renderData;
		renderData.m_World = transform.MatTransform;
		renderData.m_LayerMask = mask;
		renderData.m_ObjectID = pScene->GetEntityUUID(entity);
		renderData.m_SceneID = pScene->GetUUID();
		renderData.m_MeshID = renderData.m_ObjectID;
		renderData.m_MaterialID = pFont->Material();

		Resource* pMeshResource = m_pResources->GetResource<MeshData>(renderData.m_MeshID);
		if (!pMeshResource)
		{
			pMeshResource = new MeshData(uint32_t(textData.m_Text.size()*4), sizeof(VertexPosColorTex),
				{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
			pMeshResource->SetResourceUUID(renderData.m_MeshID);
			m_pResources->AddResource(&pMeshResource);
			textData.m_TextDirty = true;
		}

		if (textData.m_TextDirty)
		{
			MeshData* pMeshData = static_cast<MeshData*>(pMeshResource);
			const float textWrap = textData.m_TextWrap*textData.m_Scale*pFont->FontHeight();
			Utils::GenerateTextMesh(pMeshData, pFont, textData, textWrap);
		}

		pRenderer->SubmitDynamic(std::move(renderData));
	}

	void TextManager::GetReferencesImpl(std::vector<UUID>& references) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const TextComponent& text = GetAt(i);
			const UUID font = text.m_Font.GetUUID();
			if (font) references.push_back(font);
		}
	}

	void TextManager::OnDeserialize(Utils::BinaryStream&)
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const TextComponent& text = GetAt(i);
			text.m_Font.ManualRegisterReference();
		}
	}

	void TextManager::UnpackDataInto(const void* data, TextComponent& newComponent)
	{
		const TextComponent* pSourceText = reinterpret_cast<const TextComponent*>(data);
		newComponent.m_Font = pSourceText->m_Font;
		newComponent.m_Text = pSourceText->m_Text;
		newComponent.m_Scale = pSourceText->m_Scale;
		newComponent.m_Color = pSourceText->m_Color;
		newComponent.m_Alignment = pSourceText->m_Alignment;
		newComponent.m_WrapWidth = pSourceText->m_WrapWidth;
	}

	void TextManager::SerializeDense(Utils::BinaryStream& stream) const
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			const TextComponent& text = GetAt(i);
			stream.Write(text.m_Font)
				.Write(text.m_Text)
				.Write(text.m_Scale)
				.Write(text.m_Color)
				.Write(text.m_Alignment)
				.Write(text.m_WrapWidth);
		}
	}

	void TextManager::DeserializeDense(Utils::BinaryStream& stream)
	{
		for (size_t i = 0; i < Size(); ++i)
		{
			TextComponent& text = GetAt(i);
			stream.Read(text.m_Font)
				.Read(text.m_Text)
				.Read(text.m_Scale)
				.Read(text.m_Color)
				.Read(text.m_Alignment)
				.Read(text.m_WrapWidth);
		}
	}

	void TextManager::OnInitialize()
	{
		Bind(DoDraw, &TextManager::OnDrawImpl);
		Bind(DoGetReferences, &TextManager::GetReferencesImpl);
	}
}
