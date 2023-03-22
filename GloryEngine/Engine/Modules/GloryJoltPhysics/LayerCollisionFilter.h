#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace Glory
{
	class JoltPhysicsModule;

	class LayerCollisionFilter : public JPH::ObjectLayerPairFilter
	{
	public:
		LayerCollisionFilter(JoltPhysicsModule* pModule);

		virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;

	private:
		const JoltPhysicsModule* m_pPhysicsModule;
	};
}

