#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Physics.h"

#include <UUID.h>
#include <ShapeProperty.h>
#include <Reflection.h>

namespace Glory
{
	struct PhysicsBody
	{
		static constexpr uint32_t InvalidBodyID = 0xffffffff;

		PhysicsBody() : m_BodyType(BodyType::Dynamic), m_BodyID(InvalidBodyID), m_CurrentLayerIndex(0) {}

		REFLECTABLE(PhysicsBody,
			(BodyType)(m_BodyType),
			(ShapeProperty)(m_Shape)
		);

		uint32_t m_BodyID;
		uint32_t m_CurrentLayerIndex;
	};

	struct CharacterController
	{
		CharacterController() : m_MaxSlopeAngle(45.0f), m_Friction(0.5f), m_Shape(), m_CharacterID(0), m_BodyID(PhysicsBody::InvalidBodyID), m_CurrentLayerIndex(0) {}

		REFLECTABLE(CharacterController,
			(float)(m_MaxSlopeAngle),
			(ShapeProperty)(m_Shape),
			(float)(m_Friction)
		);

		uint32_t m_CharacterID;
		uint32_t m_BodyID;
		uint32_t m_CurrentLayerIndex;
		UUID m_ShapeID;
	};
}
