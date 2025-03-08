#pragma once
#include <EntityID.h>

#include <functional>
#include <map>
#include <memory>

namespace Glory::Utils::ECS
{
	class EntityRegistry;
}

namespace Glory
{
	struct UITransform;
	struct UIImage;
	struct UIText;
	struct UIConstraint;

	class UITransformSystem
	{
	public:
		static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent);

	private:
		static void CalculateMatrix(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UITransform& pComponent);
	};

	class UIImageSystem
	{
	public:
		static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIImage& pComponent);
		static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIImage& pComponent);
	};

	class UITextSystem
	{
	public:
		static void OnDraw(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent);
		static void OnDirty(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIText& pComponent);
	};

	/*class UIConstraintSystem
	{
	public:
		static void OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, UIConstraint& pComponent);
	};*/
}
