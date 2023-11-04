#include "LayerCollisionFilter.h"

#include "JoltPhysicsModule.h"

namespace Glory
{
	LayerCollisionFilter::LayerCollisionFilter()
		: m_pPhysicsModule(nullptr) {}

	bool LayerCollisionFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
	{
		return m_pPhysicsModule->ShouldCollide(inObject1, inObject2);
	}
}
