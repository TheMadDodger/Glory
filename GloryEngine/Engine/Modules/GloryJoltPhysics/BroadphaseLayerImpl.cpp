#include "BroadphaseLayerImpl.h"

using namespace JPH;

namespace Glory
{
	BPLayerInterfaceImpl::BPLayerInterfaceImpl()
	{
		m_ObjectToBroadPhase.push_back(JPH::BroadPhaseLayer(0));
	}

	JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
	{
		return m_ObjectToBroadPhase.size();
	}

	JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
	{
		if (inLayer >= m_ObjectToBroadPhase.size())
			return m_ObjectToBroadPhase[0];
		return m_ObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const
	{
		switch ((BroadPhaseLayer::Type)inLayer)
		{
		case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	void BPLayerInterfaceImpl::SetObjectToBroadphase(std::vector<JPH::BroadPhaseLayer>&& mapping)
	{
		m_ObjectToBroadPhase = std::move(mapping);
	}
}