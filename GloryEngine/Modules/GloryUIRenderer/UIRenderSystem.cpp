#include "UIRenderSystem.h"
#include "UIRendererModule.h"
#include "UIComponents.h"

#include <EntityRegistry.h>
#include <SceneManager.h>
#include <GScene.h>
#include <Engine.h>
#include <Components.h>

namespace Glory
{
	void UIRenderSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIRenderer& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		UIRendererModule* pModule = pEngine->GetOptionalModule<UIRendererModule>();

		UIRenderData data;
		data.m_DocumentID = pComponent.m_Document.AssetUUID();
		data.m_ObjectID = pScene->GetEntityUUID(entity);
		data.m_TargetCamera = 0;

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

		pModule->Submit(std::move(data));
	}
}
