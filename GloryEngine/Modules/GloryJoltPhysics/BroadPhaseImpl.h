#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhase.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
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

	// An example contact listener
	class MyContactListener : public JPH::ContactListener
	{
	public:
		MyContactListener(JoltPhysicsModule* pPhysics) : m_pPhysics(pPhysics) {}

		// See: ContactListener
		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg baseOffset, const JPH::CollideShapeResult& collisionResult) override;
		virtual void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) override;
		virtual void OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) override;
		virtual void OnContactRemoved(const JPH::SubShapeIDPair& subShapePair) override;

	private:
		JoltPhysicsModule* m_pPhysics;
	};

	// An example activation listener
	class MyBodyActivationListener : public JPH::BodyActivationListener
	{
	public:
		MyBodyActivationListener(JoltPhysicsModule* pPhysics) : m_pPhysics(pPhysics) {}

		virtual void OnBodyActivated(const JPH::BodyID& bodyID, JPH::uint64 bodyUserData) override;
		virtual void OnBodyDeactivated(const JPH::BodyID& bodyID, JPH::uint64 bodyUserData) override;

	private:
		JoltPhysicsModule* m_pPhysics;
	};
}
