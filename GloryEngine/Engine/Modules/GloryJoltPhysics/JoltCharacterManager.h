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

        uint32_t CreateCharacter_Internal(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& inPosition, const glm::quat& inRotation, const Shape& shape, float friction) override;
        void DestroyCharacter_Internal(uint32_t characterID) override;
        void DestroyAll_Internal() override;

        glm::vec3 CharacterPosition(uint32_t characterID) override;
        glm::quat CharacterRotation(uint32_t characterID) override;

    private:
        friend class JoltPhysicsModule;
        JPH::PhysicsSystem* m_pPhysicsSystem;
        std::map<uint32_t, JPH::Character*> m_pCharacters;
    };
}
