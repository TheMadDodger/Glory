#include "BroadPhaseImpl.h"
#include "JoltPhysicsModule.h"
#include <LayerManager.h>

using namespace JPH;

namespace Glory
{
	BPLayerInterfaceImpl::BPLayerInterfaceImpl() : m_ObjectToBroadPhase()
	{
	}

	JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
	{
		return (JPH::uint)Enum<BPLayer>().NumValues();
	}

	JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer layer) const
	{
		if (m_ObjectToBroadPhase.find(layer) == m_ObjectToBroadPhase.end())
			return JPH::BroadPhaseLayer(0);
		return m_ObjectToBroadPhase.at(layer);
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(BroadPhaseLayer layer) const
	{
		return "UNKNOWN";
	}
#endif

	void BPLayerInterfaceImpl::SetObjectToBroadphase(std::map<JPH::ObjectLayer, JPH::BroadPhaseLayer>&& mapping)
	{
		m_ObjectToBroadPhase = std::move(mapping);
	}

	bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const
	{
		if (layer1 == 0) return true;
		const uint8_t index = layer2.operator JPH::BroadPhaseLayer::Type();
		const Layer* pLayer = LayerManager::GetLayerAtIndex(layer1 - 1);
		if (!pLayer) return true;
		if (!m_BPCollissionMapping[index]) return true;
		return m_BPCollissionMapping[index] & pLayer->m_Mask;
	}

	void ObjectVsBroadPhaseLayerFilterImpl::SetBPCollisionMapping(std::vector<LayerMask>&& mapping)
	{
		m_BPCollissionMapping = std::move(mapping);
	}

	JPH::ValidateResult MyContactListener::OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg baseOffset, const JPH::CollideShapeResult& collisionResult)
	{
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	void MyContactListener::OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings)
	{
		m_pPhysics->TriggerLateContactCallback(ContactCallback::Added, body1.GetID().GetIndexAndSequenceNumber(), body2.GetID().GetIndexAndSequenceNumber());
	}

	void MyContactListener::OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings)
	{
		m_pPhysics->TriggerLateContactCallback(ContactCallback::Persisted, body1.GetID().GetIndexAndSequenceNumber(), body2.GetID().GetIndexAndSequenceNumber());
	}

	void MyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& subShapePair)
	{
		m_pPhysics->TriggerLateContactCallback(ContactCallback::Removed, subShapePair.GetBody1ID().GetIndexAndSequenceNumber(), subShapePair.GetBody2ID().GetIndexAndSequenceNumber());
	}

	void MyBodyActivationListener::OnBodyActivated(const JPH::BodyID& bodyID, JPH::uint64 bodyUserData)
	{
		m_pPhysics->TriggerLateActivationCallback(ActivationCallback::Activated, bodyID.GetIndexAndSequenceNumber());
	}

	void MyBodyActivationListener::OnBodyDeactivated(const JPH::BodyID& bodyID, JPH::uint64 bodyUserData)
	{
		m_pPhysics->TriggerLateActivationCallback(ActivationCallback::Deactivated, bodyID.GetIndexAndSequenceNumber());
	}
}
