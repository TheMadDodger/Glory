#include "CharacterControllerSystem.h"
#include <glm/gtx/matrix_decompose.hpp>

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

		const Transform& transform = pRegistry->GetComponent<Transform>(entity);
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 scale;
		glm::vec3 skew;
		glm::vec4 perspective;
		if (!glm::decompose(transform.MatTransform, scale, rotation, translation, skew, perspective)) return;
		pComponent.m_CurrentLayerIndex = pRegistry->HasComponent<LayerComponent>(entity) ? pRegistry->GetComponent<LayerComponent>(entity).m_Layer.m_LayerIndex : 0;
		pComponent.m_CharacterID = pCharacters->CreateCharacter(pComponent.m_MaxSlopeAngle, pComponent.m_CurrentLayerIndex, translation, rotation, *pComponent.m_Shape.BaseShapePointer(), pComponent.m_Friction);
	}

	void CharacterControllerSystem::OnStop(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent)
	{
		if (!pComponent.m_CharacterID) return;

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

		pCharacters->DestroyCharacter(pComponent.m_CharacterID);
		pComponent.m_CharacterID = 0;
	}

	void CharacterControllerSystem::OnValidate(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent)
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
	}

	void CharacterControllerSystem::OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, CharacterController& pComponent)
	{
		if (!pComponent.m_CharacterID) return;

		PhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetPhysicsModule();
		if (!pPhysics) return;

		CharacterManager* pCharacters = pPhysics->GetCharacterManager();
		if (!pCharacters) return;
		
		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		transform.Position = pCharacters->GetPosition(pComponent.m_CharacterID);
		transform.Rotation = pCharacters->GetRotation(pComponent.m_CharacterID);
		transform.Rotation = glm::conjugate(transform.Rotation);

		if (transform.Parent.IsValid())
		{
			Transform& parentTransform = transform.Parent.GetComponent<Transform>();
			const glm::mat4 inverse = glm::inverse(parentTransform.MatTransform);
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			if (!glm::decompose(inverse, scale, rotation, translation, skew, perspective)) return;
			transform.Position = inverse * glm::vec4(transform.Position, 1.0f);
			transform.Rotation = transform.Rotation * glm::inverse(rotation);
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
