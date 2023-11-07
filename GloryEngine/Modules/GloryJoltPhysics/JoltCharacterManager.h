#pragma once
#include "Physics.h"

#include <map>
#include <cstdint>
#include <glm/gtc/quaternion.hpp>
#include <Glory.h>

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
        GLORY_API uint32_t CreateCharacter(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& inPosition, const glm::quat& inRotation, const ShapeData& shape, float friction);
        GLORY_API void DestroyCharacter(uint32_t characterID);
        GLORY_API void DestroyAll();

		GLORY_API void Activate(uint32_t characterID, bool lockBodies = true);
		GLORY_API void SetLinearAndAngularVelocity(uint32_t characterID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity, bool lockBodies = true);
		GLORY_API glm::vec3 GetLinearVelocity(uint32_t characterID, bool lockBodies = true) const;
		GLORY_API void SetLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies = true);
		GLORY_API void AddLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies = true);
		GLORY_API void AddImpulse(uint32_t characterID, const glm::vec3& impulse, bool lockBodies = true);
		GLORY_API uint32_t GetBodyID(uint32_t characterID) const;
		GLORY_API void GetPositionAndRotation(uint32_t characterID, glm::vec3& outPosition, glm::quat& outRotation, bool lockBodies = true) const;
		GLORY_API void SetPositionAndRotation(uint32_t characterID, const glm::vec3& position, const glm::quat& rotation, ActivationType activationMode, bool lockBodies = true) const;
		GLORY_API glm::vec3 GetPosition(uint32_t characterID, bool lockBodies = true) const;
		GLORY_API void SetPosition(uint32_t characterID, const glm::vec3& position, ActivationType activationMode, bool lockBodies = true);
		GLORY_API glm::quat GetRotation(uint32_t characterID, bool lockBodies = true) const;
		GLORY_API void SetRotation(uint32_t characterID, const glm::quat& rotation, ActivationType activationMode, bool lockBodies = true);
		GLORY_API glm::vec3 GetCenterOfMassPosition(uint32_t characterID, bool lockBodies = true) const;
		GLORY_API void SetLayer(uint32_t characterID, uint32_t layer, bool lockBodies = true);
		GLORY_API bool SetShape(uint32_t characterID, const ShapeData& shape, float maxPenetrationDepth, bool lockBodies = true);

    private:
        void SetPhysicsSystem(JPH::PhysicsSystem* pPhysics);

    private:
        friend class JoltPhysicsModule;
		void PostSimulation(float maxSeparationDistance, bool lockBodies = true);

        JPH::PhysicsSystem* m_pPhysicsSystem;
        std::map<uint32_t, JPH::Character*> m_pCharacters;
    };
}
