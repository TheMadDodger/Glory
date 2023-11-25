#include "CharacterControllerSystem.h"
#include "JoltPhysicsModule.h"
#include "JoltComponents.h"
#include "JoltCharacterManager.h"
#include "JoltShapeManager.h"
#include "JoltPhysicsModule.h"
#include "PhysicsSystem.h"

#include <Engine.h>
#include <Game.h>
#include <GScene.h>

#include <Components.h>
#include <Engine.h>
#include <glm/gtx/matrix_decompose.hpp>

namespace Glory
{
	void CharacterControllerSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CharacterController& pComponent)
	{
		pComponent.m_BodyID = PhysicsBody::InvalidBodyID;
		JoltPhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetMainModule<JoltPhysicsModule>();
		JoltCharacterManager* pCharacters = pPhysics->GetCharacterManager();
		JoltShapeManager* pShapes = pPhysics->GetShapeManager();

		const Shape* pShape = pComponent.m_Shape.BaseShapePointer();
		if (pShape->m_ShapeType == ShapeType::None)
		{
			Debug::LogWarning("CharacterController does not have a shape!");
			return;
		}

		const Transform& transform = pRegistry->GetComponent<Transform>(entity);
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 scale;
		glm::vec3 skew;
		glm::vec4 perspective;
		if (!glm::decompose(transform.MatTransform, scale, rotation, translation, skew, perspective)) return;

		const UUID shapeID = pShapes->CreateShape(*pShape);
		const ShapeData* pShapeData = pShapes->GetShape(shapeID);

		pComponent.m_CurrentLayerIndex = pRegistry->HasComponent<LayerComponent>(entity) ? pRegistry->GetComponent<LayerComponent>(entity).m_Layer.m_LayerIndex : 0;
		pComponent.m_ShapeID = shapeID;
		pComponent.m_CharacterID = pCharacters->CreateCharacter(pComponent.m_MaxSlopeAngle, pComponent.m_CurrentLayerIndex, translation, rotation, *pShapeData, pComponent.m_Friction);
		pComponent.m_BodyID = pCharacters->GetBodyID(pComponent.m_CharacterID);
		PhysicsSystem::AddBody(pComponent.m_BodyID, pRegistry, entity);
	}

	void CharacterControllerSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CharacterController& pComponent)
	{
		if (!pComponent.m_CharacterID) return;

		JoltPhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetMainModule<JoltPhysicsModule>();
		JoltCharacterManager* pCharacters = pPhysics->GetCharacterManager();

		PhysicsSystem::RemoveBody(pComponent.m_BodyID);
		pCharacters->DestroyCharacter(pComponent.m_CharacterID);
		pComponent.m_CharacterID = 0;
		pComponent.m_BodyID = 0;
		pComponent.m_ShapeID = 0;
	}

	void CharacterControllerSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CharacterController& pComponent)
	{
		JoltPhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetMainModule<JoltPhysicsModule>();
		if (!pPhysics)
		{
			Debug::LogWarning("A CharacterController was added to an entity but no PhysicsModule was loaded");
			return;
		}

		JoltCharacterManager* pCharacters = pPhysics->GetCharacterManager();
	}

	void CharacterControllerSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, CharacterController& pComponent)
	{
		if (!pComponent.m_CharacterID) return;

		JoltPhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetMainModule<JoltPhysicsModule>();

		JoltCharacterManager* pCharacters = pPhysics->GetCharacterManager();
		if (!pCharacters) return;
		
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		transform.Position = pCharacters->GetPosition(pComponent.m_CharacterID);
		transform.Rotation = pCharacters->GetRotation(pComponent.m_CharacterID);

		Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
		Entity parent = pRegistry->GetUserData<GScene*>()->GetEntityByUUID(pEntityView->Parent());
		if (parent.IsValid())
		{
			Transform& parentTransform = parent.GetComponent<Transform>();
			const glm::mat4 inverse = glm::inverse(parentTransform.MatTransform);
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			if (!glm::decompose(inverse, scale, rotation, translation, skew, perspective)) return;
			transform.Position = inverse * glm::vec4(transform.Position, 1.0f);
			transform.Rotation = transform.Rotation * glm::conjugate(rotation);
		}

		if (pRegistry->HasComponent<LayerComponent>(entity))
		{
			const uint32_t layerIndex = pRegistry->GetComponent<LayerComponent>(entity).m_Layer.m_LayerIndex;
			if (pComponent.m_CurrentLayerIndex != layerIndex)
			{
				pCharacters->SetLayer(pComponent.m_CharacterID, layerIndex);
				pComponent.m_CurrentLayerIndex = layerIndex;
			}
		}
		else
		{
			if (pComponent.m_CurrentLayerIndex != 0)
			{
				pCharacters->SetLayer(pComponent.m_CharacterID, 0);
				pComponent.m_CurrentLayerIndex = 0;
			}
		}
	}
}
