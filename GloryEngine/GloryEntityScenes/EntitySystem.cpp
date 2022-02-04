#include "EntitySystem.h"

namespace Glory
{
	EntitySystem::EntitySystem(Registry* pRegistry, const std::type_index& type) : m_pRegistry(pRegistry), m_ComponentType(type) {}

	EntitySystem::~EntitySystem()
	{
	}

	bool EntitySystem::Hidden()
	{
		return m_Hidden;
	}

	std::string EntitySystem::GetPath()
	{
		return "";
	}

	std::string EntitySystem::Name()
	{
		return "Unknown Component";
	}

	std::type_index EntitySystem::GetComponentType() const
	{
		return m_ComponentType;
	}
}
