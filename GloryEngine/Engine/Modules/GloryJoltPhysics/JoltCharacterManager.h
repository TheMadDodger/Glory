#pragma once
#include <CharacterManager.h>
#include <map>

namespace JPH
{
    class PhysicsSystem;
    class Character;
}

namespace Glory
{
    class JoltCharacterManager : public CharacterManager
    {
    private:
        void SetPhysicsSystem(JPH::PhysicsSystem* pPhysics);

        uint32_t CreateCharacter_Internal(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& inPosition, const glm::quat& inRotation, const ShapeData& shape, float friction) override;
        void DestroyCharacter_Internal(uint32_t characterID) override;
        void DestroyAll_Internal() override;

		void Activate(uint32_t characterID, bool lockBodies) override;
		void SetLinearAndAngularVelocity(uint32_t characterID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity, bool lockBodies) override;
		glm::vec3 GetLinearVelocity(uint32_t characterID, bool lockBodies) const override;
		void SetLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies) override;
		void AddLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies) override;
		void AddImpulse(uint32_t characterID, const glm::vec3& impulse, bool lockBodies) override;
		uint32_t GetBodyID(uint32_t characterID) const override;
		void GetPositionAndRotation(uint32_t characterID, glm::vec3& outPosition, glm::quat& outRotation, bool lockBodies) const override;
		void SetPositionAndRotation(uint32_t characterID, const glm::vec3& position, const glm::quat& rotation, ActivationType activationMode, bool lockBodies) const override;
		glm::vec3 GetPosition(uint32_t characterID, bool lockBodies) const override;
		void SetPosition(uint32_t characterID, const glm::vec3& position, ActivationType activationMode, bool lockBodies) override;
		glm::quat GetRotation(uint32_t characterID, bool lockBodies) const override;
		void SetRotation(uint32_t characterID, const glm::quat& rotation, ActivationType activationMode, bool lockBodies) override;
		glm::vec3 GetCenterOfMassPosition(uint32_t characterID, bool lockBodies) const override;
		void SetLayer(uint32_t characterID, uint32_t layer, bool lockBodies) override;
		bool SetShape(uint32_t characterID, const ShapeData& shape, float maxPenetrationDepth, bool lockBodies) override;

    private:
        friend class JoltPhysicsModule;
		void PostSimulation(float maxSeparationDistance, bool lockBodies = true);

        JPH::PhysicsSystem* m_pPhysicsSystem;
        std::map<uint32_t, JPH::Character*> m_pCharacters;
    };
}
