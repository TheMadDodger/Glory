#include "TextSystem.h"
#include "Components.h"

#include "GScene.h"
#include "RendererModule.h"
#include "SceneManager.h"
#include "Engine.h"
#include "Components.h"
#include "RenderData.h"

#include <TypeView.h>
#include <EntityRegistry.h>

namespace Glory
{
	void TextSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
		if (!pComponent.m_Font || pComponent.m_Text.empty()) return;

		GScene* pScene = pRegistry->GetUserData<GScene*>();
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

		TextRenderData renderData;
		renderData.m_FontID = pComponent.m_Font.AssetUUID();
		renderData.m_World = transform.MatTransform;
		renderData.m_LayerMask = mask;
		renderData.m_ObjectID = pScene->GetEntityUUID(entity);
		renderData.m_SceneID = pScene->GetUUID();
		renderData.m_Text = pComponent.m_Text;
		renderData.m_TextDirty = pComponent.m_Dirty;
		renderData.m_Scale = pComponent.m_Scale;
		renderData.m_Alignment = pComponent.m_Alignment;
		renderData.m_TextWrap = pComponent.m_WrapWidth;
		renderData.m_Color = pComponent.m_Color;
		pComponent.m_Dirty = false;

		REQUIRE_MODULE_CALL(pEngine, RendererModule, SubmitDynamic(std::move(renderData)), );
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
