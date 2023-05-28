#include "JoltCharacterManager.h"
#include "Helpers.h"

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

	uint32_t JoltCharacterManager::CreateCharacter_Internal(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& inPosition, const glm::quat& inRotation, const Shape& shape, float friction)
	{
		uint32_t characterID = uint32_t(UUID());

		constexpr float cCharacterRadiusStanding = 1.0f;

		JPH::CharacterSettings settings = {};
		settings.mMaxSlopeAngle = JPH::DegreesToRadians(maxSlopeAngle);
		settings.mLayer = JPH::ObjectLayer(layerIndex);
		settings.mShape = GetJPHShape(shape);
		settings.mFriction = friction;
		settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
		JPH::Character* pCharacter = new JPH::Character(&settings, ToJPHVec3(inPosition), ToJPHQuat(inRotation), 0, m_pPhysicsSystem);
		pCharacter->AddToPhysicsSystem(JPH::EActivation::Activate);

		m_pCharacters.emplace(characterID, pCharacter);
		return characterID;
	}

	void JoltCharacterManager::DestroyCharacter_Internal(uint32_t characterID)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->RemoveFromPhysicsSystem();
		delete itor->second;
		m_pCharacters.erase(itor);
	}

	void JoltCharacterManager::DestroyAll_Internal()
	{
		for (auto itor : m_pCharacters)
		{
			delete itor.second;
		}
		m_pCharacters.clear();
	}

	glm::vec3 JoltCharacterManager::CharacterPosition(uint32_t characterID)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return {};
		return ToVec3(itor->second->GetPosition());
	}

	glm::quat JoltCharacterManager::CharacterRotation(uint32_t characterID)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return {};
		return ToQuat(itor->second->GetRotation());
	}
}
