#include "CharacterControllerSystem.h"

namespace Glory
{
	void CharacterControllerSystem::OnStart(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent)
	{
		PhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetPhysicsModule();
		if (!pPhysics)
		{
			Debug::LogWarning("A CharacterController was added to an entity but no PhysicsModule was loaded");
			return;
		}

		CharacterManager* pCharacters = pPhysics->GetCharacterManager();
		if (!pCharacters)
		{
			Debug::LogWarning("The loaded PhysicsModule does not have support for CharacterControllers");
			return;
		}

		pCharacters->CreateCharacter();
	}

	void CharacterControllerSystem::OnStop(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent)
	{
	}

	void CharacterControllerSystem::OnValidate(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent)
	{
	}

	void CharacterControllerSystem::OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent)
	{
	}
}
