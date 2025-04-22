#include "UIRenderSystem.h"
#include "UIRendererModule.h"
#include "UIComponents.h"
#include "UIDocument.h"

#include <EntityRegistry.h>
#include <SceneManager.h>
#include <GScene.h>
#include <Engine.h>
#include <InputModule.h>
#include <Components.h>

namespace Glory
{
	void UIRenderSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIRenderer& pComponent)
	{
		if (!pComponent.m_RenderDocumentID) return;
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		UIRendererModule* pModule = pEngine->GetOptionalModule<UIRendererModule>();
		UIDocument* pDocument = pModule->FindDocument(pComponent.m_RenderDocumentID);
		if (!pDocument) return;
		pDocument->Start();
	}

	void UIRenderSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIRenderer& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		UIRendererModule* pModule = pEngine->GetOptionalModule<UIRendererModule>();

		UIDocumentData* pDocument = pComponent.m_Document.Get(&pEngine->GetAssetManager());
		if (!pDocument) return;

		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		UIRenderData data;
		data.m_DocumentID = pComponent.m_Document.AssetUUID();
		data.m_ObjectID = pScene->GetEntityUUID(entity);
		data.m_SceneID = pScene->GetUUID();
		data.m_TargetCamera = 0;
		data.m_WorldTransform = transform.MatTransform;
		data.m_Target = pComponent.m_Target;
		data.m_InputEnabled = pComponent.m_InputEnabled;
		pComponent.m_RenderDocumentID = data.m_ObjectID;
		pComponent.m_IsDirty = true;
		data.m_WorldDirty = pComponent.m_IsDirty;

		glm::uvec2 resolution = pComponent.m_Resolution;

		switch (pComponent.m_Target)
		{
		case UITarget::CameraOverlay:
		{
			if (!pRegistry->HasComponent<CameraComponent>(entity))
			{
				pEngine->GetDebug().LogError("Can't overlay UI on camera, entity has no camera component");
				return;
			}
			CameraComponent& camera = pRegistry->GetComponent<CameraComponent>(entity);
			data.m_TargetCamera = camera.m_Camera.GetUUID();
			resolution = camera.m_Camera.GetResolution();
			break;
		}
		default:
			break;
		}

		switch (pComponent.m_ResolutionMode)
		{
		case ResolutionMode::CameraScale:
			if (pComponent.m_Target != UITarget::CameraOverlay)
			{
				pEngine->GetDebug().LogError("Target not set to CameraOverlay, can't base resolution on camera");
				return;
			}
			data.m_Resolution = glm::uvec2(resolution.x*pComponent.m_Resolution.x, resolution.y*pComponent.m_Resolution.y);
			break;
		case ResolutionMode::Fixed:
			data.m_Resolution = resolution;
			break;
		default:
			break;
		}

		pModule->Create(data, pDocument);
	}

	void UIRenderSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIRenderer& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		UIRendererModule* pModule = pEngine->GetOptionalModule<UIRendererModule>();

		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		UIRenderData data;
		data.m_DocumentID = pComponent.m_Document.AssetUUID();
		data.m_ObjectID = pScene->GetEntityUUID(entity);
		data.m_SceneID = pScene->GetUUID();
		data.m_TargetCamera = 0;
		data.m_WorldTransform = transform.MatTransform;
		data.m_Target = pComponent.m_Target;
		data.m_MaterialID = pComponent.m_WorldMaterial.AssetUUID();
		data.m_WorldSize = pComponent.m_WorldSize;
		data.m_WorldDirty = pComponent.m_IsDirty;
		data.m_InputEnabled = pComponent.m_InputEnabled;
		pComponent.m_RenderDocumentID = data.m_ObjectID;
		pComponent.m_IsDirty = false;

		glm::uvec2 resolution = pComponent.m_Resolution;

		switch (pComponent.m_Target)
		{
		case UITarget::CameraOverlay:
		{
			if (!pRegistry->HasComponent<CameraComponent>(entity))
				return;
			CameraComponent& camera = pRegistry->GetComponent<CameraComponent>(entity);
			data.m_TargetCamera = camera.m_Camera.GetUUID();
			resolution = camera.m_Camera.GetResolution();

			InputModule* pInput = pEngine->GetMainModule<InputModule>();
			if (pInput)
			{
				const glm::vec2 screenScale = 1.0f / pInput->GetScreenScale();
				pComponent.m_CursorPos = pInput->GetCursorPos(0)*screenScale;
				pComponent.m_CursorDown = pInput->IsCursorDown(0);
			}
			break;
		}
		case UITarget::WorldSpaceQuad:
		{
			LayerManager* pLayers = &pEngine->GetLayerManager();
			LayerMask mask;
			if (pRegistry->HasComponent<LayerComponent>(entity))
			{
				LayerComponent& layer = pRegistry->GetComponent<LayerComponent>(entity);
				mask = layer.m_Layer.Layer(pLayers) != nullptr ? layer.m_Layer.Layer(pLayers)->m_Mask : 0;
			}
			data.m_LayerMask = mask;
			break;
		}
		default:
			break;
		}

		switch (pComponent.m_ResolutionMode)
		{
		case ResolutionMode::CameraScale:
			if (pComponent.m_Target != UITarget::CameraOverlay)
				return;
			data.m_Resolution = glm::uvec2(resolution.x*pComponent.m_Resolution.x, resolution.y*pComponent.m_Resolution.y);
			break;
		case ResolutionMode::Fixed:
			data.m_Resolution = resolution;
			break;
		default:
			break;
		}

		data.m_CursorPos = pComponent.m_CursorPos;
		data.m_CursorDown = pComponent.m_CursorDown;

		pModule->Submit(std::move(data));
	}

	void UIRenderSystem::GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references)
	{
		for (size_t i = 0; i < pTypeView->Size(); ++i)
		{
			const UIRenderer* pMeshRenderer = static_cast<const UIRenderer*>(pTypeView->GetComponentAddressFromIndex(i));
			const UUID document = pMeshRenderer->m_Document.AssetUUID();
			const UUID material = pMeshRenderer->m_WorldMaterial.AssetUUID();
			if (document) references.push_back(document);
			if (material) references.push_back(material);
		}
	}
}
