#pragma once
#include <EntityID.h>
#include <UUID.h>

#include <functional>
#include <map>
#include <memory>
#include <Glory.h>

namespace Glory::Utils::ECS
{
	class EntityRegistry;
	class BaseTypeView;
}

namespace Glory
{
	struct UITransform;
	struct UIImage;
	struct UIText;
	struct UIBox;
	struct UIInteraction;
	struct UIPanel;
	struct UIConstraint;
	struct UIVerticalContainer;
	struct UIScrollView;
	class Engine;

	class UITransformSystem
	{
	public:
		static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent);
		static void CalculateMatrix(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent, bool calculateParentIfDirty=true);
	};

	class UIImageSystem
	{
	public:
		static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIImage& pComponent);
		static void GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references);
	};

	class UITextSystem
	{
	public:
		static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent);
		static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent);
		static void OnDirty(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent);
		static void GetReferences(const Utils::ECS::BaseTypeView* pTypeView, std::vector<UUID>& references);
	};
	
	class UIBoxSystem
	{
	public:
		static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIBox& pComponent);
	};
	
	class UIInteractionSystem
	{
	public:
		static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIInteraction& pComponent);

		static GLORY_API UIInteractionSystem* Instance();

	public:
		/* Engine, SceneID, ObjectID, ElementID, ComponentID */
		std::function<void(Engine*, UUID, UUID, UUID, UUID)> OnElementHover_Callback;
		/* Engine, SceneID, ObjectID, ElementID, ComponentID */
		std::function<void(Engine*, UUID, UUID, UUID, UUID)> OnElementUnHover_Callback;
		/* Engine, SceneID, ObjectID, ElementID, ComponentID */
		std::function<void(Engine*, UUID, UUID, UUID, UUID)> OnElementDown_Callback;
		/* Engine, SceneID, ObjectID, ElementID, ComponentID */
		std::function<void(Engine*, UUID, UUID, UUID, UUID)> OnElementUp_Callback;
	};

	class UIPanelSystem
	{
	public:
		static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIPanel& pComponent);
		static void OnPostDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIPanel& pComponent);
	};

	class UIVerticalContainerSystem
	{
	public:
		static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIVerticalContainer& pComponent);
		static void OnDirty(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIVerticalContainer& pComponent);
	};
	
	class UIScrollViewSystem
	{
	public:
		static void OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIScrollView& pComponent);
		static void OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIScrollView& pComponent);
		static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIScrollView& pComponent);
		static void OnDirty(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIScrollView& pComponent);
	};
}
