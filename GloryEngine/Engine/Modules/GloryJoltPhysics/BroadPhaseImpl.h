#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhase.h>
#include <Glory.h>
#include <LayerMask.h>

REFLECTABLE_ENUM_NS(Glory, BPLayer, Default, One, Two, Three);

namespace Glory
{
	class JoltPhysicsModule;

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadphase layers.
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl();
		JPH::uint GetNumBroadPhaseLayers() const override;
		JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer layer) const override;
#endif

		GLORY_API void SetObjectToBroadphase(std::map<JPH::ObjectLayer, JPH::BroadPhaseLayer>&& mapping);

	private:
		std::map<JPH::ObjectLayer, JPH::BroadPhaseLayer> m_ObjectToBroadPhase;
	};

	/// Class that determines if an object layer can collide with a broadphase layer
	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override;

		GLORY_API void SetBPCollisionMapping(std::vector<LayerMask>&& mapping);

	private:
		std::vector<LayerMask> m_BPCollissionMapping;
	};
}
