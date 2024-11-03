#include "PhysicsComponentsCSAPI.h"
#include "JoltPhysicsModule.h"
#include "JoltCharacterManager.h"
#include "JoltShapeManager.h"

#include <PhysicsComponents.h>
#include <EntityCSAPI.h>
#include <cstdint>
#include <UUID.h>
#include <GScene.h>
#include <SceneManager.h>
#include <MathCSAPI.h>
#include <GloryMonoScipting.h>
#include <Debug.h>
#include <Engine.h>

namespace Glory
{
	Engine* PhysicsComponents_EngineInstance;

	template<typename T>
	static T& GetComponent(UUID sceneID, UUID objectID, uint64_t componentID)
	{
		GScene* pScene = GetEntityScene(sceneID);
		Entity entity = pScene->GetEntityByUUID(objectID);
		Utils::ECS::EntityView* pEntityView = entity.GetEntityView();
		uint32_t hash = pEntityView->ComponentType(componentID);
		return pScene->GetRegistry().GetComponent<T>(entity.GetEntityID());
	}

#pragma region Physics Component

#define PHYSICS PhysicsComponents_EngineInstance->GetOptionalModule<JoltPhysicsModule>()

#pragma region States

	uint32_t PhysicsBody_GetID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		return physicsComp.m_BodyID;
	}

	void PhysicsBody_Activate(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->ActivateBody(physicsComp.m_BodyID);
	}

	void PhysicsBody_Deactivate(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->DeactivateBody(physicsComp.m_BodyID);
	}

	bool PhysicsBody_IsActive(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		return PHYSICS->IsBodyActive(physicsComp.m_BodyID);
	}

	bool PhysicsBody_IsValid(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		return PHYSICS->IsValidBody(physicsComp.m_BodyID);
	}

#pragma endregion

#pragma region Position and rotation

	void PhysicsBody_SetPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* position, const ActivationType activationType)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->SetBodyPosition(physicsComp.m_BodyID, ToGLMVec3(*position), activationType);
	}

	void PhysicsBody_SetRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const QuatWrapper* rotation, const ActivationType activationType)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->SetBodyRotation(physicsComp.m_BodyID, ToGLMQuat(*rotation), activationType);
	}

	void PhysicsBody_SetScale(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* scale, const ActivationType activationType)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->SetBodyScale(physicsComp.m_BodyID, ToGLMVec3(*scale), activationType);
	}

	Vec3Wrapper PhysicsBody_GetPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		return PHYSICS->GetBodyPosition(physicsComp.m_BodyID);
	}

	Vec3Wrapper PhysicsBody_GetCenterOfMassPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		return PHYSICS->GetBodyCenterOfMassPosition(physicsComp.m_BodyID);
	}

	QuatWrapper PhysicsBody_GetRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		return PHYSICS->GetBodyRotation(physicsComp.m_BodyID);
	}

#pragma endregion

#pragma region Velocities

	void PhysicsBody_MoveKinematic(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* targetPosition, const QuatWrapper* targetRotation, float deltaTime)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->MoveBodyKinematic(physicsComp.m_BodyID, ToGLMVec3(*targetPosition), ToGLMQuat(*targetRotation), deltaTime);
	}

	void PhysicsBody_SetLinearAndAngularVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->SetBodyLinearAndAngularVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void PhysicsBody_GetLinearAndAngularVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, Vec3Wrapper* linearVelocity, Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		glm::vec3 lv, av;
		PHYSICS->GetBodyLinearAndAngularVelocity(physicsComp.m_BodyID, lv, av);
		*linearVelocity = lv;
		*angularVelocity = av;
	}

	void PhysicsBody_SetLinearVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* linearVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->SetBodyLinearVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity));
	}

	Vec3Wrapper PhysicsBody_GetLinearVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		return PHYSICS->GetBodyLinearVelocity(physicsComp.m_BodyID);
	}

	void PhysicsBody_AddLinearVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* linearVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->AddBodyLinearVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity));
	}

	void PhysicsBody_AddLinearAndAngularVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->AddBodyLinearAndAngularVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void PhysicsBody_SetAngularVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->SetBodyAngularVelocity(physicsComp.m_BodyID, ToGLMVec3(*angularVelocity));
	}

	Vec3Wrapper PhysicsBody_GetAngularVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		return PHYSICS->GetBodyAngularVelocity(physicsComp.m_BodyID);
	}

	Vec3Wrapper PhysicsBody_GetPointVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* point)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		return PHYSICS->GetBodyPointVelocity(physicsComp.m_BodyID, ToGLMVec3(*point));
	}

	void PhysicsBody_SetPositionRotationAndVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* position, const QuatWrapper* rotation, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->SetBodyPositionRotationAndVelocity(physicsComp.m_BodyID, ToGLMVec3(*position), ToGLMQuat(*rotation), ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

#pragma endregion

#pragma region Forces

	void PhysicsBody_AddForce(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* force)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->AddBodyForce(physicsComp.m_BodyID, ToGLMVec3(*force));
	}

	void PhysicsBody_AddForce_Point(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* force, const Vec3Wrapper* point)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->AddBodyForce(physicsComp.m_BodyID, ToGLMVec3(*force), ToGLMVec3(*point));
	}

	void PhysicsBody_AddTorque(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* torque)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->AddBodyTorque(physicsComp.m_BodyID, ToGLMVec3(*torque));
	}

	void PhysicsBody_AddForceAndTorque(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* force, const Vec3Wrapper* torque)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->AddBodyForceAndTorque(physicsComp.m_BodyID, ToGLMVec3(*force), ToGLMVec3(*torque));
	}

#pragma endregion

#pragma region Impulses

	void PhysicsBody_AddImpulse(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* impulse)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->AddBodyImpulse(physicsComp.m_BodyID, ToGLMVec3(*impulse));
	}

	void PhysicsBody_AddImpulse_Point(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* impulse, const Vec3Wrapper* point)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->AddBodyForce(physicsComp.m_BodyID, ToGLMVec3(*impulse), ToGLMVec3(*point));
	}

	void PhysicsBody_AddAngularImpulse(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* angularImpulse)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(sceneID, objectID, componentID);
		PHYSICS->AddBodyAngularImpulse(physicsComp.m_BodyID, ToGLMVec3(*angularImpulse));
	}

#pragma endregion

#pragma endregion

#pragma region Character Controller

#define CHARACTERS PHYSICS->GetCharacterManager()
#define SHAPES PHYSICS->GetShapeManager()

#pragma region States

	uint32_t CharacterController_GetCharacterID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		return characterController.m_CharacterID;
	}

	uint32_t CharacterController_GetBodyID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		return CHARACTERS->GetBodyID(characterController.m_CharacterID);
	}

	void CharacterController_Activate(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		CHARACTERS->Activate(characterController.m_CharacterID);
	}

	void CharacterController_Deactivate(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		const uint32_t bodyID = CHARACTERS->GetBodyID(characterController.m_CharacterID);
		PHYSICS->DeactivateBody(bodyID);
	}

	bool CharacterController_IsActive(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		const uint32_t bodyID = CHARACTERS->GetBodyID(characterController.m_CharacterID);
		return PHYSICS->IsBodyActive(bodyID);
	}

#pragma endregion

#pragma region Position and rotation

	void CharacterController_SetPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* position, const ActivationType activationType)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		CHARACTERS->SetPosition(characterController.m_CharacterID, ToGLMVec3(*position), activationType);
	}

	void CharacterController_SetRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const QuatWrapper* rotation, const ActivationType activationType)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		CHARACTERS->SetRotation(characterController.m_CharacterID, ToGLMQuat(*rotation), activationType);
	}

	Vec3Wrapper CharacterController_GetPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		return CHARACTERS->GetPosition(characterController.m_CharacterID);
	}

	Vec3Wrapper CharacterController_GetCenterOfMassPosition(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		return CHARACTERS->GetCenterOfMassPosition(characterController.m_CharacterID);
	}

	QuatWrapper CharacterController_GetRotation(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		return CHARACTERS->GetRotation(characterController.m_CharacterID);
	}

#pragma endregion

#pragma region Velocities

	void CharacterController_SetLinearAndAngularVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		CHARACTERS->SetLinearAndAngularVelocity(characterController.m_CharacterID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void CharacterController_SetLinearVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* linearVelocity)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		CHARACTERS->SetLinearVelocity(characterController.m_CharacterID, ToGLMVec3(*linearVelocity));
	}

	Vec3Wrapper CharacterController_GetLinearVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		return CHARACTERS->GetLinearVelocity(characterController.m_CharacterID);
	}

	void CharacterController_AddLinearVelocity(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* linearVelocity)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		CHARACTERS->AddLinearVelocity(characterController.m_CharacterID, ToGLMVec3(*linearVelocity));
	}

#pragma endregion

#pragma region Impulses

	void CharacterController_AddImpulse(uint64_t sceneID, uint64_t objectID, uint64_t componentID, const Vec3Wrapper* impulse)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		CHARACTERS->AddImpulse(characterController.m_CharacterID, ToGLMVec3(*impulse));
	}

#pragma endregion

#pragma region Shapes

	uint64_t CharacterController_GetShapeID(uint64_t sceneID, uint64_t objectID, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		return characterController.m_ShapeID;
	}

	bool CharacterController_SetShape(uint64_t sceneID, uint64_t objectID, uint64_t componentID, uint64_t shapeID, float maxPenetrationDepth = 0.0f, bool lockBodies = true)
	{
		CharacterController& characterController = GetComponent<CharacterController>(sceneID, objectID, componentID);
		const ShapeData* pShapeData = SHAPES->GetShape(shapeID);
		if (!pShapeData)
		{
			std::stringstream log;
			log << "CharacterController_SetShape: Failed to set shape, shape with ID: " << shapeID << " does not exist!";
			PhysicsComponents_EngineInstance->GetDebug().LogError(log.str());
			return false;
		}

		/* Never continue to store the original shape! Some physics engines like jolt delete them upon swappin! */
		SHAPES->DestroyShape(characterController.m_ShapeID);
		characterController.m_ShapeID = shapeID;
		return CHARACTERS->SetShape(characterController.m_CharacterID, *pShapeData, maxPenetrationDepth, lockBodies);
	}

#pragma endregion

#pragma endregion

#pragma region Binding

	void PhysicsComponentsCSAPI::AddInternalCalls(std::vector<InternalCall>& internalCalls)
	{
		/* PhysicsBody */
		/* status */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetID", PhysicsBody_GetID);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_Activate", PhysicsBody_Activate);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_Deactivate", PhysicsBody_Deactivate);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_IsActive", PhysicsBody_IsActive);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_IsValid", PhysicsBody_IsValid);

		/* Position and rotation */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetPosition", PhysicsBody_SetPosition);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetRotation", PhysicsBody_SetRotation);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetScale", PhysicsBody_SetScale);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetPosition", PhysicsBody_GetPosition);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetCenterOfMassPosition", PhysicsBody_GetCenterOfMassPosition);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetRotation", PhysicsBody_GetRotation);

		/* Velocities */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_MoveKinematic", PhysicsBody_MoveKinematic);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetLinearAndAngularVelocity", PhysicsBody_SetLinearAndAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetLinearAndAngularVelocity", PhysicsBody_GetLinearAndAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetLinearVelocity", PhysicsBody_SetLinearVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetLinearVelocity", PhysicsBody_GetLinearVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddLinearVelocity", PhysicsBody_AddLinearVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddLinearAndAngularVelocity", PhysicsBody_AddLinearAndAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetAngularVelocity", PhysicsBody_SetAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetAngularVelocity", PhysicsBody_GetAngularVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_GetPointVelocity", PhysicsBody_GetPointVelocity);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_SetPositionRotationAndVelocity", PhysicsBody_SetPositionRotationAndVelocity);

		/* Forces */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddForce", PhysicsBody_AddForce);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddForce_Point", PhysicsBody_AddForce_Point);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddTorque", PhysicsBody_AddTorque);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddForceAndTorque", PhysicsBody_AddForceAndTorque);

		/* Impulses */
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddImpulse", PhysicsBody_AddImpulse);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddImpulse_Point", PhysicsBody_AddImpulse_Point);
		BIND("GloryEngine.Entities.PhysicsBody::PhysicsBody_AddAngularImpulse", PhysicsBody_AddAngularImpulse);

		/* Status */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetCharacterID", CharacterController_GetCharacterID);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetBodyID", CharacterController_GetBodyID);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_Activate", CharacterController_Activate);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_Deactivate", CharacterController_Deactivate);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_IsActive", CharacterController_IsActive);

		/* Position and rotation */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetPosition", CharacterController_SetPosition);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetRotation", CharacterController_SetRotation);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetPosition", CharacterController_GetPosition);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetCenterOfMassPosition", CharacterController_GetCenterOfMassPosition);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetRotation", CharacterController_GetRotation);

		/* Velocities */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetLinearAndAngularVelocity", CharacterController_SetLinearAndAngularVelocity);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetLinearVelocity", CharacterController_SetLinearVelocity);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetLinearVelocity", CharacterController_GetLinearVelocity);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_AddLinearVelocity", CharacterController_AddLinearVelocity);

		/* Shape */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_SetShape", CharacterController_SetShape);
		BIND("GloryEngine.Entities.CharacterController::CharacterController_GetShapeID", CharacterController_GetShapeID);

		/* Impulses */
		BIND("GloryEngine.Entities.CharacterController::CharacterController_AddImpulse", CharacterController_AddImpulse);
	}

	void PhysicsComponentsCSAPI::SetEngine(Engine* pEngine)
	{
		PhysicsComponents_EngineInstance = pEngine;
	}

#pragma endregion

}