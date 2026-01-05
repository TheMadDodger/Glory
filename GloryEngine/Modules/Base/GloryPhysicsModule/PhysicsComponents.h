#pragma once
#include "Physics.h"

#include <UUID.h>
#include <ShapeProperty.h>
#include <Reflection.h>

namespace Glory
{
	struct PhysicsSimulationSettings
	{
		PhysicsSimulationSettings() : m_DegreesOfFreedom(AllowedDOFFlag::All), m_AllowDynamicOrKinematic(false), m_IsSensor(false),
			m_SensorDetectsStatic(false), m_UseManifoldReduction(true), m_MotionQuality(MotionQuality::Discrete),
			m_AllowSleeping(true), m_Friction(0.2f), m_Restitution(0.0f), m_LinearDamping(0.05f), 
			m_AngularDamping(0.05f), m_MaxLinearVelocity(500.0f), m_MaxAngularVelocity(2700.0f),
			m_GravityFactor(1.0f) {}

		REFLECTABLE(PhysicsSimulationSettings,
			(AllowedDOFFlag)(m_DegreesOfFreedom),
			(bool)(m_AllowDynamicOrKinematic),
			(bool)(m_IsSensor),
			(bool)(m_SensorDetectsStatic),
			(bool)(m_UseManifoldReduction),
			(MotionQuality)(m_MotionQuality),
			(bool)(m_AllowSleeping),
			(float)(m_Friction),
			(float)(m_Restitution),
			(float)(m_LinearDamping),
			(float)(m_AngularDamping),
			(float)(m_MaxLinearVelocity),
			(float)(m_MaxAngularVelocity),
			(float)(m_GravityFactor)
		);
	};

	struct PhysicsBody
	{
		static constexpr uint32_t InvalidBodyID = 0xffffffff;

		PhysicsBody() : m_BodyType(BodyType::Dynamic), m_BodyID(InvalidBodyID), m_CurrentLayerIndex(0) {}

		REFLECTABLE(PhysicsBody,
			(BodyType)(m_BodyType),
			(ShapeProperty)(m_Shape),
			(PhysicsSimulationSettings)(m_SimulationSettings)
		);

		uint32_t m_BodyID;
		uint32_t m_CurrentLayerIndex;
	};

	struct CharacterController
	{
		CharacterController() : m_MaxSlopeAngle(45.0f), m_Mass(80.0f), m_Friction(0.2f),
			m_GravityFactor(1.0f), m_Shape(), m_CharacterID(0),
			m_BodyID(PhysicsBody::InvalidBodyID), m_CurrentLayerIndex(0), m_ShapeID(0) {}

		REFLECTABLE(CharacterController,
			(float)(m_MaxSlopeAngle),
			(ShapeProperty)(m_Shape),
			(float)(m_Mass),
			(float)(m_Friction),
			(float)(m_GravityFactor)
		);

		uint32_t m_CharacterID;
		uint32_t m_BodyID;
		uint32_t m_CurrentLayerIndex;
		UUID m_ShapeID;
	};
}
