#include "LayerCollisionFilter.h"

#include "JoltPhysicsModule.h"

namespace Glory
{
	LayerCollisionFilter::LayerCollisionFilter(JoltPhysicsModule* pModule)
		: m_pPhysicsModule(pModule) {}

	bool LayerCollisionFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
	{
		return m_pPhysicsModule->ShouldCollide(inObject1, inObject2);
	}
}
