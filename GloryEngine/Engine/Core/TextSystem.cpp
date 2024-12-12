#include "TextSystem.h"
#include "Components.h"

#include "GScene.h"
#include "RendererModule.h"
#include "EntityRegistry.h"
#include "SceneManager.h"
#include "Engine.h"
#include "Components.h"
#include "RenderData.h"

namespace Glory
{
	void TextSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
		if (!pComponent.m_Font) return;

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
		pComponent.m_Dirty = false;

		REQUIRE_MODULE_CALL(pEngine, RendererModule, Submit(std::move(renderData)), );
	}
}
