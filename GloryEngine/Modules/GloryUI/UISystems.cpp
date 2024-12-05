#include "UISystems.h"
#include "GloryUIModule.h"
#include "UIComponents.h"

#include <GScene.h>
#include <EntityRegistry.h>
#include <SceneManager.h>
#include <Engine.h>
#include <Components.h>
#include <RenderData.h>

namespace Glory
{
	void TextSystem::OnRemove(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
	}

	void TextSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
	}

	void TextSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();
		GloryUIModule* pUIModule = pEngine->GetOptionalModule<GloryUIModule>();

		if (!pComponent.m_Font) return;

		FontData* pFont = pComponent.m_Font.Get(&pEngine->GetAssetManager());
		if (!pFont) return;

		pUIModule->ReserveFontSize(pFont, pComponent.m_Size);
	}

	void TextSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
	}

	void TextSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
	}

	void TextSystem::OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, TextComponent& pComponent)
	{
	}
}
