#include "CharacterManager.h"

namespace Glory
{
	uint32_t CharacterManager::CreateCharacter(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& inPosition, const glm::quat& inRotation, const Shape& shape, float friction)
	{
		return CreateCharacter_Internal(maxSlopeAngle, layerIndex, inPosition, inRotation, shape, friction);
	}

	void CharacterManager::DestroyCharacter(uint32_t characterID)
	{
		DestroyCharacter_Internal(characterID);
	}

	void CharacterManager::DestroyAll()
	{
		DestroyAll_Internal();
	}
}
