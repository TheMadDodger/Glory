#include "TextSystem.h"

#include "Components.h"
#include "GScene.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "Engine.h"
#include "VertexHelpers.h"
#include "RenderData.h"
#include "AssetManager.h"

#include <TypeView.h>
#include <EntityRegistry.h>

namespace Glory
{
	void TextSystem::OnDisableDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		const UUID meshID = pScene->GetEntityUUID(entity);
		pEngine->GetAssetManager().UnloadAsset(meshID);
	}

	void TextSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
		if (!pComponent.m_Font || pComponent.m_Text.empty()) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Renderer* pRenderer = pScene->Manager()->GetRenderer();
		if (!pRenderer) return;

		Engine* pEngine = pScene->Manager()->GetEngine();
		LayerManager* pLayers = &pEngine->GetLayerManager();

		FontData* pFont = pComponent.m_Font.Get(&pEngine->GetAssetManager());
		if (!pFont) return;

		Transform& transform = pRegistry->GetComponent<Transform>(entity);

		LayerMask mask;
		if (pRegistry->HasComponent<LayerComponent>(entity))
		{
			LayerComponent& layer = pRegistry->GetComponent<LayerComponent>(entity);
			mask = layer.m_Layer.Layer(pLayers) != nullptr ? layer.m_Layer.Layer(pLayers)->m_Mask : 0;
		}

		const UUID fontID = pComponent.m_Font.AssetUUID();

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

		Resource* pMeshResource = pEngine->GetAssetManager().FindResource(renderData.m_MeshID);
		if (!pMeshResource)
		{
			pMeshResource = new MeshData(textData.m_Text.size()*4, sizeof(VertexPosColorTex),
				{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 });
			pMeshResource->SetResourceUUID(renderData.m_MeshID);
			pEngine->GetAssetManager().AddLoadedResource(pMeshResource);
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

	void TextSystem::GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references)
	{
		for (size_t i = 0; i < pTypeView->Size(); ++i)
		{
			const TextComponent* pText = static_cast<const TextComponent*>(pTypeView->GetComponentAddressFromIndex(i));
			const UUID font = pText->m_Font.AssetUUID();
			if (font) references.push_back(font);
		}
	}
}
