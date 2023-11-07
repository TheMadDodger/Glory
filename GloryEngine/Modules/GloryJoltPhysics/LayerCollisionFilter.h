#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace Glory
{
	class JoltPhysicsModule;

	class LayerCollisionFilter : public JPH::ObjectLayerPairFilter
	{
	public:
		LayerCollisionFilter();

		virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;

	private:
		friend class JoltPhysicsModule;
		const JoltPhysicsModule* m_pPhysicsModule;
	};
}

