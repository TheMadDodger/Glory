#include "JoltShapeManager.h"
#include "JoltCharacterManager.h"
#include "Helpers.h"

#include <Shapes.h>
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

	uint32_t JoltCharacterManager::CreateCharacter(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& position, const glm::quat& rotation, const ShapeData& shape, float friction)
	{
		uint32_t characterID = uint32_t(UUID());

		constexpr float cCharacterRadiusStanding = 1.0f;

		JPH::CharacterSettings settings = {};
		settings.mMaxSlopeAngle = JPH::DegreesToRadians(maxSlopeAngle);
		settings.mLayer = JPH::ObjectLayer(layerIndex);
		settings.mShape = (JPH::Shape*)shape.m_pShape;
		settings.mFriction = friction;
		settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
		JPH::Character* pCharacter = new JPH::Character(&settings, ToJPHVec3(position), ToJPHQuat(rotation), 0, m_pPhysicsSystem);
		pCharacter->AddToPhysicsSystem(JPH::EActivation::Activate);

		m_pCharacters.emplace(characterID, pCharacter);
		return characterID;
	}

	void JoltCharacterManager::DestroyCharacter(uint32_t characterID)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->RemoveFromPhysicsSystem();
		delete itor->second;
		m_pCharacters.erase(itor);
	}

	void JoltCharacterManager::DestroyAll()
	{
		for (auto itor : m_pCharacters)
		{
			delete itor.second;
		}
		m_pCharacters.clear();
	}

	void JoltCharacterManager::Activate(uint32_t characterID, bool lockBodies)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->Activate(lockBodies);
	}

	void JoltCharacterManager::SetLinearAndAngularVelocity(uint32_t characterID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity, bool lockBodies)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->SetLinearAndAngularVelocity(ToJPHVec3(linearVelocity), ToJPHVec3(angularVelocity), lockBodies);
	}

	glm::vec3 JoltCharacterManager::GetLinearVelocity(uint32_t characterID, bool lockBodies) const
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return {};
		return ToVec3(itor->second->GetLinearVelocity(lockBodies));
	}

	void JoltCharacterManager::SetLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->SetLinearVelocity(ToJPHVec3(linearVelocity), lockBodies);
	}

	void JoltCharacterManager::AddLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->AddLinearVelocity(ToJPHVec3(linearVelocity), lockBodies);
	}

	void JoltCharacterManager::AddImpulse(uint32_t characterID, const glm::vec3& impulse, bool lockBodies)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->AddImpulse(ToJPHVec3(impulse), lockBodies);
	}

	uint32_t JoltCharacterManager::GetBodyID(uint32_t characterID) const
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return 0;
		return itor->second->GetBodyID().GetIndexAndSequenceNumber();
	}

	void JoltCharacterManager::GetPositionAndRotation(uint32_t characterID, glm::vec3& outPosition, glm::quat& outRotation, bool lockBodies) const
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		JPH::Vec3 position;
		JPH::Quat rotation;
		itor->second->GetPositionAndRotation(position, rotation, lockBodies);
		outPosition = ToVec3(position);
		outRotation = ToQuat(rotation);
	}

	void JoltCharacterManager::SetPositionAndRotation(uint32_t characterID, const glm::vec3& position, const glm::quat& rotation, ActivationType activationMode, bool lockBodies) const
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->SetPositionAndRotation(ToJPHVec3(position), ToJPHQuat(rotation), JPH::EActivation(activationMode), lockBodies);
	}

	glm::vec3 JoltCharacterManager::GetPosition(uint32_t characterID, bool lockBodies) const
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return {};
		return ToVec3(itor->second->GetPosition());
	}

	void JoltCharacterManager::SetPosition(uint32_t characterID, const glm::vec3& position, ActivationType activationMode, bool lockBodies)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->SetPosition(ToJPHVec3(position), JPH::EActivation(activationMode), lockBodies);
	}

	glm::quat JoltCharacterManager::GetRotation(uint32_t characterID, bool lockBodies) const
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return {};
		return ToQuat(itor->second->GetRotation());
	}

	void JoltCharacterManager::SetRotation(uint32_t characterID, const glm::quat& rotation, ActivationType activationMode, bool lockBodies)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->SetRotation(ToJPHQuat(rotation), JPH::EActivation(activationMode), lockBodies);
	}

	glm::vec3 JoltCharacterManager::GetCenterOfMassPosition(uint32_t characterID, bool lockBodies) const
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return {};
		return ToVec3(itor->second->GetCenterOfMassPosition(lockBodies));
	}

	void JoltCharacterManager::SetLayer(uint32_t characterID, uint32_t layer, bool lockBodies)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return;
		itor->second->SetLayer(JPH::ObjectLayer(layer), lockBodies);
	}

	bool JoltCharacterManager::SetShape(uint32_t characterID, const ShapeData& shape, float maxPenetrationDepth, bool lockBodies)
	{
		auto itor = m_pCharacters.find(characterID);
		if (itor == m_pCharacters.end()) return false;
		JPH::Shape* pShape = (JPH::Shape*)shape.m_pShape;
		return itor->second->SetShape(pShape, maxPenetrationDepth, lockBodies);
	}

	void JoltCharacterManager::PostSimulation(float maxSeparationDistance, bool lockBodies)
	{
		for (auto itor : m_pCharacters)
		{
			itor.second->PostSimulation(maxSeparationDistance, lockBodies);
		}
	}
}
