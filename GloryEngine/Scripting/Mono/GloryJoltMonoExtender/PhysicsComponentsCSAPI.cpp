#include "JoltPhysicsModule.h"
#include "PhysicsComponentsCSAPI.h"
#include "JoltComponents.h"

#include <EntityCSAPI.h>
#include <cstdint>
#include <UUID.h>
#include <GScene.h>
#include <SceneManager.h>
#include <MathCSAPI.h>
#include <GloryMonoScipting.h>

namespace Glory
{
	GScene* GetEntityScene(UUID sceneID)
	{
		if (sceneID == 0) return nullptr;
		GScene* pScene = Game::GetGame().GetEngine()->GetSceneManager()->GetOpenScene(sceneID);
		if (pScene == nullptr) return nullptr;
		return pScene;
	}

	GScene* GetEntityScene(MonoEntityHandle* pEntityHandle)
	{
		if (pEntityHandle->m_EntityID == 0) return nullptr;
		return GetEntityScene((UUID)pEntityHandle->m_SceneID);
	}

	template<typename T>
	static T& GetComponent(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		GScene* pScene = GetEntityScene(pEntityHandle);
		Utils::ECS::EntityView* pEntityView = pScene->GetRegistry()->GetEntityView(pEntityHandle->m_EntityID);
		uint32_t hash = pEntityView->ComponentType(componentID);
		return pScene->GetRegistry()->GetComponent<T>(pEntityHandle->m_EntityID);
	}

#pragma region Physics Component

#define PHYSICS Game::GetGame().GetEngine()->GetMainModule<JoltPhysicsModule>()

#pragma region States

	uint32_t PhysicsBody_GetID(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return physicsComp.m_BodyID;
	}

	void PhysicsBody_Activate(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->ActivateBody(physicsComp.m_BodyID);
	}

	void PhysicsBody_Deactivate(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->DeactivateBody(physicsComp.m_BodyID);
	}

	bool PhysicsBody_IsActive(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->IsBodyActive(physicsComp.m_BodyID);
	}

	bool PhysicsBody_IsValid(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->IsValidBody(physicsComp.m_BodyID);
	}

#pragma endregion

#pragma region Position and rotation

	void PhysicsBody_SetPosition(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* position, const ActivationType activationType)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyPosition(physicsComp.m_BodyID, ToGLMVec3(*position), activationType);
	}

	void PhysicsBody_SetRotation(MonoEntityHandle* pEntityHandle, UUID componentID, const QuatWrapper* rotation, const ActivationType activationType)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyRotation(physicsComp.m_BodyID, ToGLMQuat(*rotation), activationType);
	}

	void PhysicsBody_SetScale(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* scale, const ActivationType activationType)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyScale(physicsComp.m_BodyID, ToGLMVec3(*scale), activationType);
	}

	Vec3Wrapper PhysicsBody_GetPosition(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyPosition(physicsComp.m_BodyID);
	}

	Vec3Wrapper PhysicsBody_GetCenterOfMassPosition(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyCenterOfMassPosition(physicsComp.m_BodyID);
	}

	QuatWrapper PhysicsBody_GetRotation(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyRotation(physicsComp.m_BodyID);
	}

#pragma endregion

#pragma region Velocities

	void PhysicsBody_MoveKinematic(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* targetPosition, const QuatWrapper* targetRotation, float deltaTime)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->MoveBodyKinematic(physicsComp.m_BodyID, ToGLMVec3(*targetPosition), ToGLMQuat(*targetRotation), deltaTime);
	}

	void PhysicsBody_SetLinearAndAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyLinearAndAngularVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void PhysicsBody_GetLinearAndAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, Vec3Wrapper* linearVelocity, Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		glm::vec3 lv, av;
		PHYSICS->GetBodyLinearAndAngularVelocity(physicsComp.m_BodyID, lv, av);
		*linearVelocity = lv;
		*angularVelocity = av;
	}

	void PhysicsBody_SetLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyLinearVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity));
	}

	Vec3Wrapper PhysicsBody_GetLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyLinearVelocity(physicsComp.m_BodyID);
	}

	void PhysicsBody_AddLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyLinearVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity));
	}

	void PhysicsBody_AddLinearAndAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyLinearAndAngularVelocity(physicsComp.m_BodyID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void PhysicsBody_SetAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyAngularVelocity(physicsComp.m_BodyID, ToGLMVec3(*angularVelocity));
	}

	Vec3Wrapper PhysicsBody_GetAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyAngularVelocity(physicsComp.m_BodyID);
	}

	Vec3Wrapper PhysicsBody_GetPointVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* point)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		return PHYSICS->GetBodyPointVelocity(physicsComp.m_BodyID, ToGLMVec3(*point));
	}

	void PhysicsBody_SetPositionRotationAndVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* position, const QuatWrapper* rotation, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->SetBodyPositionRotationAndVelocity(physicsComp.m_BodyID, ToGLMVec3(*position), ToGLMQuat(*rotation), ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

#pragma endregion

#pragma region Forces

	void PhysicsBody_AddForce(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* force)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyForce(physicsComp.m_BodyID, ToGLMVec3(*force));
	}

	void PhysicsBody_AddForce_Point(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* force, const Vec3Wrapper* point)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyForce(physicsComp.m_BodyID, ToGLMVec3(*force), ToGLMVec3(*point));
	}

	void PhysicsBody_AddTorque(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* torque)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyTorque(physicsComp.m_BodyID, ToGLMVec3(*torque));
	}

	void PhysicsBody_AddForceAndTorque(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* force, const Vec3Wrapper* torque)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyForceAndTorque(physicsComp.m_BodyID, ToGLMVec3(*force), ToGLMVec3(*torque));
	}

#pragma endregion

#pragma region Impulses

	void PhysicsBody_AddImpulse(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* impulse)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyImpulse(physicsComp.m_BodyID, ToGLMVec3(*impulse));
	}

	void PhysicsBody_AddImpulse_Point(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* impulse, const Vec3Wrapper* point)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyForce(physicsComp.m_BodyID, ToGLMVec3(*impulse), ToGLMVec3(*point));
	}

	void PhysicsBody_AddAngularImpulse(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* angularImpulse)
	{
		PhysicsBody& physicsComp = GetComponent<PhysicsBody>(pEntityHandle, componentID);
		PHYSICS->AddBodyAngularImpulse(physicsComp.m_BodyID, ToGLMVec3(*angularImpulse));
	}

#pragma endregion

#pragma endregion

#pragma region Character Controller

#define CHARACTERS PHYSICS->GetCharacterManager()
#define SHAPES PHYSICS->GetShapeManager()

#pragma region States

	uint32_t CharacterController_GetCharacterID(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return characterController.m_CharacterID;
	}

	uint32_t CharacterController_GetBodyID(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetBodyID(characterController.m_CharacterID);
	}

	void CharacterController_Activate(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->Activate(characterController.m_CharacterID);
	}

	void CharacterController_Deactivate(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		const uint32_t bodyID = CHARACTERS->GetBodyID(characterController.m_CharacterID);
		PHYSICS->DeactivateBody(bodyID);
	}

	bool CharacterController_IsActive(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		const uint32_t bodyID = CHARACTERS->GetBodyID(characterController.m_CharacterID);
		return PHYSICS->IsBodyActive(bodyID);
	}

#pragma endregion

#pragma region Position and rotation

	void CharacterController_SetPosition(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* position, const ActivationType activationType)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->SetPosition(characterController.m_CharacterID, ToGLMVec3(*position), activationType);
	}

	void CharacterController_SetRotation(MonoEntityHandle* pEntityHandle, UUID componentID, const QuatWrapper* rotation, const ActivationType activationType)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->SetRotation(characterController.m_CharacterID, ToGLMQuat(*rotation), activationType);
	}

	Vec3Wrapper CharacterController_GetPosition(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetPosition(characterController.m_CharacterID);
	}

	Vec3Wrapper CharacterController_GetCenterOfMassPosition(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetCenterOfMassPosition(characterController.m_CharacterID);
	}

	QuatWrapper CharacterController_GetRotation(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetRotation(characterController.m_CharacterID);
	}

#pragma endregion

#pragma region Velocities

	void CharacterController_SetLinearAndAngularVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity, const Vec3Wrapper* angularVelocity)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->SetLinearAndAngularVelocity(characterController.m_CharacterID, ToGLMVec3(*linearVelocity), ToGLMVec3(*angularVelocity));
	}

	void CharacterController_SetLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->SetLinearVelocity(characterController.m_CharacterID, ToGLMVec3(*linearVelocity));
	}

	Vec3Wrapper CharacterController_GetLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return CHARACTERS->GetLinearVelocity(characterController.m_CharacterID);
	}

	void CharacterController_AddLinearVelocity(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* linearVelocity)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->AddLinearVelocity(characterController.m_CharacterID, ToGLMVec3(*linearVelocity));
	}

#pragma endregion

#pragma region Impulses

	void CharacterController_AddImpulse(MonoEntityHandle* pEntityHandle, UUID componentID, const Vec3Wrapper* impulse)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		CHARACTERS->AddImpulse(characterController.m_CharacterID, ToGLMVec3(*impulse));
	}

#pragma endregion

#pragma region Shapes

	uint64_t CharacterController_GetShapeID(MonoEntityHandle* pEntityHandle, uint64_t componentID)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		return characterController.m_ShapeID;
	}

	bool CharacterController_SetShape(MonoEntityHandle* pEntityHandle, uint64_t componentID, uint64_t shapeID, float maxPenetrationDepth = 0.0f, bool lockBodies = true)
	{
		CharacterController& characterController = GetComponent<CharacterController>(pEntityHandle, componentID);
		const ShapeData* pShapeData = SHAPES->GetShape(shapeID);
		if (!pShapeData)
		{
			std::stringstream log;
			log << "CharacterController_SetShape: Failed to set shape, shape with ID: " << shapeID << " does not exist!";
			Debug::LogError(log.str());
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

#pragma endregion

}