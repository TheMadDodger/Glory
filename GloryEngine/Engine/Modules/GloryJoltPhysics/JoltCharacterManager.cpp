#include "JoltCharacterManager.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Character/Character.h>
#include <UUID.h>

namespace Glory
{
	void JoltCharacterManager::SetPhysicsSystem(JPH::PhysicsSystem* pPhysics)
	{
		m_pPhysicsSystem = pPhysics;
	}

	void JoltCharacterManager::CreateCharacter_Internal()
	{
		uint32_t characterID = uint32_t(UUID());

		float cCharacterRadiusStanding = 1.0f;

		JPH::CharacterSettings settings = {};
		settings.mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
		settings.mLayer = 0;
		settings.mShape = mStandingShape;
		settings.mFriction = 0.5f;
		settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
		JPH::Character* pCharacter = new JPH::Character(&settings, JPH::RVec3::sZero(), JPH::Quat::sIdentity(), 0, m_pPhysicsSystem);
		pCharacter->AddToPhysicsSystem(JPH::EActivation::Activate);

		m_pCharacters.emplace(characterID, pCharacter);
	}
}
