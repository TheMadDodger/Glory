#pragma once
#include "Shapes.h"
#include <cstdint>

namespace Glory
{
	class CharacterManager
	{
	public:
		uint32_t CreateCharacter(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& inPosition, const glm::quat& inRotation, const Shape& shape, float friction);
		void DestroyCharacter(uint32_t characterID);

		void DestroyAll();

		virtual glm::vec3 CharacterPosition(uint32_t characterID) = 0;
		virtual glm::quat CharacterRotation(uint32_t characterID) = 0;

	protected:
		virtual uint32_t CreateCharacter_Internal(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& inPosition, const glm::quat& inRotation, const Shape& shape, float friction) = 0;
		virtual void DestroyCharacter_Internal(uint32_t) = 0;
		virtual void DestroyAll_Internal() = 0;
	};
}
