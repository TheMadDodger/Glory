#pragma once
#include "EntityID.h"

#include <vector>

namespace Glory::Utils::ECS
{
	class IComponentManager
	{
	public:
		virtual ~IComponentManager() = default;

	public:
		virtual uint32_t ComponentHash() const = 0;
		virtual void* Add(EntityID entity) = 0;
		virtual void Remove(EntityID entity) = 0;

		//virtual void OnAdd() = 0;
		//virtual void OnRemove() = 0;
		//virtual void OnValidate() = 0;
		//virtual void OnEnable() = 0;
		//virtual void OnDisable() = 0;
		//virtual void OnEnableDraw() = 0;
		//virtual void OnDisableDraw() = 0;
		//virtual void OnDirty() = 0;

		virtual void Sort(const std::vector<std::vector<EntityID>>& entityTrees) = 0;

		virtual void Initialize() = 0;
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void PreUpdate(float dt) = 0;
		virtual void Update(float dt) = 0;
		virtual void PostUpdate(float dt) = 0;
		virtual void PreDraw() = 0;
		virtual void Draw() = 0;
		virtual void PostDraw() = 0;
	};
}