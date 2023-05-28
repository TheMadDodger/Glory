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

        virtual void CreateCharacter_Internal() override;

    private:
        friend class JoltPhysicsModule;
        JPH::PhysicsSystem* m_pPhysicsSystem;
        std::map<uint32_t, JPH::Character*> m_pCharacters;
    };
}
