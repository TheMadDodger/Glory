#include "EntitySystem.h"

namespace Glory
{
	EntitySystem::EntitySystem(Registry* pRegistry, const std::type_index& type) : m_pRegistry(pRegistry), m_ComponentType(type) {}

	EntitySystem::~EntitySystem()
	{
	}
}
