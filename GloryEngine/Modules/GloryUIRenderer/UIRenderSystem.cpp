#include "UIRenderSystem.h"
#include "UIRendererModule.h"
#include "UIComponents.h"

#include <EntityRegistry.h>
#include <SceneManager.h>
#include <GScene.h>
#include <Engine.h>
#include <InputModule.h>
#include <Components.h>

namespace Glory
{
	void UIRenderSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIRenderer& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		UIRendererModule* pModule = pEngine->GetOptionalModule<UIRendererModule>();

		UIDocumentData* pDocument = pComponent.m_Document.Get(&pEngine->GetAssetManager());
		if (!pDocument) return;

		UIRenderData data;
		data.m_DocumentID = pComponent.m_Document.AssetUUID();
		data.m_ObjectID = pScene->GetEntityUUID(entity);
		data.m_SceneID = pScene->GetUUID();
		data.m_TargetCamera = 0;
		pComponent.m_RenderDocumentID = data.m_ObjectID;

		glm::uvec2 resolution = pComponent.m_Resolution;

		switch (pComponent.m_Target)
		{
		case UITarget::CameraOverlay:
		{
			if (!pRegistry->HasComponent<CameraComponent>(entity))
			{
				pEngine->GetDebug().LogOnce("UIOverlay", "Can't overlay UI on camera, entity has no camera component", Debug::LogLevel::Error);
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

		UIRenderData data;
		data.m_DocumentID = pComponent.m_Document.AssetUUID();
		data.m_ObjectID = pScene->GetEntityUUID(entity);
		data.m_SceneID = pScene->GetUUID();
		data.m_TargetCamera = 0;
		pComponent.m_RenderDocumentID = data.m_ObjectID;

		glm::uvec2 resolution = pComponent.m_Resolution;

		switch (pComponent.m_Target)
		{
		case UITarget::CameraOverlay:
		{
			if (!pRegistry->HasComponent<CameraComponent>(entity))
			{
				pEngine->GetDebug().LogOnce("UIOverlay", "Can't overlay UI on camera, entity has no camera component", Debug::LogLevel::Error);
				return;
			}
			CameraComponent& camera = pRegistry->GetComponent<CameraComponent>(entity);
			data.m_TargetCamera = camera.m_Camera.GetUUID();
			resolution = camera.m_Camera.GetResolution();

			InputModule* pInput = pEngine->GetMainModule<InputModule>();
			if (pInput)
			{
				const glm::vec2 screenScale = 1.0f/pInput->GetScreenScale();
				pComponent.m_CursorPos = pInput->GetCursorPos(0)*screenScale;
				pComponent.m_CursorDown = pInput->IsCursorDown(0);
			}
			break;
		}
		default:
			break;
		}

		switch (pComponent.m_ResolutionMode)
		{
		case ResolutionMode::CameraScale:
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
			if (document) references.push_back(document);
		}
	}
}
