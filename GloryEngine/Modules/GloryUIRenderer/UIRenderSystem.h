#pragma once
#include <EntityID.h>
#include <UUID.h>

namespace Glory::Utils::ECS
{
	class EntityRegistry;
}

namespace Glory
{
	struct UIRenderer;

	class UIRenderSystem
	{
	public:
		static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIRenderer& pComponent);
		static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIRenderer& pComponent);
	};
}
