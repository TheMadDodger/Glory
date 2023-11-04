#pragma once
#include "Physics.h"

#include <map>
#include <cstdint>
#include <glm/gtc/quaternion.hpp>

namespace JPH
{
    class PhysicsSystem;
    class Character;
}

namespace Glory
{
	struct ShapeData;

    class JoltCharacterManager
    {
	public:
        uint32_t CreateCharacter(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& inPosition, const glm::quat& inRotation, const ShapeData& shape, float friction);
        void DestroyCharacter(uint32_t characterID);
        void DestroyAll();

		void Activate(uint32_t characterID, bool lockBodies = true);
		void SetLinearAndAngularVelocity(uint32_t characterID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity, bool lockBodies = true);
		glm::vec3 GetLinearVelocity(uint32_t characterID, bool lockBodies = true) const;
		void SetLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies = true);
		void AddLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies = true);
		void AddImpulse(uint32_t characterID, const glm::vec3& impulse, bool lockBodies = true);
		uint32_t GetBodyID(uint32_t characterID) const;
		void GetPositionAndRotation(uint32_t characterID, glm::vec3& outPosition, glm::quat& outRotation, bool lockBodies = true) const;
		void SetPositionAndRotation(uint32_t characterID, const glm::vec3& position, const glm::quat& rotation, ActivationType activationMode, bool lockBodies = true) const;
		glm::vec3 GetPosition(uint32_t characterID, bool lockBodies = true) const;
		void SetPosition(uint32_t characterID, const glm::vec3& position, ActivationType activationMode, bool lockBodies = true);
		glm::quat GetRotation(uint32_t characterID, bool lockBodies = true) const;
		void SetRotation(uint32_t characterID, const glm::quat& rotation, ActivationType activationMode, bool lockBodies = true);
		glm::vec3 GetCenterOfMassPosition(uint32_t characterID, bool lockBodies = true) const;
		void SetLayer(uint32_t characterID, uint32_t layer, bool lockBodies = true);
		bool SetShape(uint32_t characterID, const ShapeData& shape, float maxPenetrationDepth, bool lockBodies = true);

    private:
        void SetPhysicsSystem(JPH::PhysicsSystem* pPhysics);

    private:
        friend class JoltPhysicsModule;
		void PostSimulation(float maxSeparationDistance, bool lockBodies = true);

        JPH::PhysicsSystem* m_pPhysicsSystem;
        std::map<uint32_t, JPH::Character*> m_pCharacters;
    };
}
