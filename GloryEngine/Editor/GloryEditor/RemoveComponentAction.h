#pragma once
#include "Undo.h"

#include <GScene.h>
#include <EntityID.h>
#include <yaml-cpp/yaml.h>

namespace Glory::Utils::ECS
{
	class EntityRegistry;
}

namespace Glory::Editor
{
	class RemoveComponentAction : public IAction
	{
	public:
		RemoveComponentAction(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entityID, size_t componentIndex);
		virtual ~RemoveComponentAction();

	private:
		virtual void OnUndo(const ActionRecord& actionRecord);
		virtual void OnRedo(const ActionRecord& actionRecord);

	private:
		std::string m_SerializedComponent;
		size_t m_ComponentIndex;
	};
}
