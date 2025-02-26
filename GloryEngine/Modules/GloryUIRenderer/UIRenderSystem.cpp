#include "UIRenderSystem.h"
#include "UIRendererModule.h"
#include "UIComponents.h"

#include <EntityRegistry.h>
#include <SceneManager.h>
#include <GScene.h>
#include <Engine.h>

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
		pModule->Submit(std::move(data));
	}
}
