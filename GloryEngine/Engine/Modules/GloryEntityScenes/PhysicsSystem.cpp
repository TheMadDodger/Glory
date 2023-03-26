#include "PhysicsSystem.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "EntitySceneScenesModule.h"

namespace Glory
{
	std::map<uint32_t, std::pair<GloryECS::EntityRegistry*, EntityID>> PhysicsSystem::m_BodyOwners;

	void PhysicsSystem::OnStart(GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent)
	{
		PhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetPhysicsModule();
		if (!pPhysics)
		{
			Debug::LogWarning("A PhysicsBody was added to an entity but no PhysocsModule was loaded");
			return;
		}

		SetupBody(pPhysics, pRegistry, entity, pComponent);
	}

	void PhysicsSystem::OnStop(GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent)
	{
		PhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetPhysicsModule();
		if (!pPhysics) return;
		if (pComponent.m_BodyID == PhysicsBody::InvalidBodyID) return;
		pPhysics->DestroyPhysicsBody(pComponent.m_BodyID);
	}

	void PhysicsSystem::OnValidate(GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent)
	{
		PhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetPhysicsModule();
		if (!pPhysics)
		{
			Debug::LogWarning("A PhysicsBody was added to an entity but no PhysocsModule was loaded");
			return;
		}

		/* If no body was created the game has not started yet */
		if(pComponent.m_BodyID == PhysicsBody::InvalidBodyID) return;

		/* Destroy the body */
		pPhysics->DestroyPhysicsBody(pComponent.m_BodyID);
		/* Create new body */
		SetupBody(pPhysics, pRegistry, entity, pComponent);
	}

	void PhysicsSystem::OnUpdate(GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent)
	{
		PhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetPhysicsModule();
		if (!pPhysics)
		{
			Debug::LogWarning("An Entity has a PhysicsBody but no PhysocsModule was loaded");
			return;
		}

		/* An invalid body cannot be updated */
		if (pComponent.m_BodyID == PhysicsBody::InvalidBodyID) return;

		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pPhysics->PollPhysicsState(pComponent.m_BodyID, &transform.Position, &transform.Rotation);
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
				pPhysics->SetBodyObjectLayer(pComponent.m_BodyID, layerIndex);
				pComponent.m_CurrentLayerIndex = layerIndex;
			}
		}
		else
		{
			if (pComponent.m_CurrentLayerIndex != 0)
			{
				pPhysics->SetBodyObjectLayer(pComponent.m_BodyID, 0);
				pComponent.m_CurrentLayerIndex = 0;
			}
		}
	}

	void PhysicsSystem::OnBodyActivated(uint32_t bodyID)
	{
		const std::pair<GloryECS::EntityRegistry*, EntityID>& pair = m_BodyOwners[bodyID];
		GloryECS::EntityRegistry* pRegistry = pair.first;
		EntityID entity = pair.second;
		if (!pRegistry->IsValid(entity)) return;
		if (!pRegistry->HasComponent<ScriptedComponent>(entity)) return;
		ScriptedComponent& scriptComponent = pRegistry->GetComponent<ScriptedComponent>(entity);
		Script* pScript = scriptComponent.m_Script.Get();
		if (!pScript) return;
		void* args[1] = {
			&bodyID
		};

		EntityScene* pEntityScene = pRegistry->GetUserData<EntityScene*>();
		pScript->Invoke(pEntityScene->GetEntitySceneObjectFromEntityID(entity), "OnBodyActivated", args);
	}

	void PhysicsSystem::OnBodyDeactivated(uint32_t bodyID)
	{
		const std::pair<GloryECS::EntityRegistry*, EntityID>& pair = m_BodyOwners[bodyID];
		GloryECS::EntityRegistry* pRegistry = pair.first;
		EntityID entity = pair.second;
		if (!pRegistry->IsValid(entity)) return;
		if (!pRegistry->HasComponent<ScriptedComponent>(entity)) return;
		ScriptedComponent& scriptComponent = pRegistry->GetComponent<ScriptedComponent>(entity);
		Script* pScript = scriptComponent.m_Script.Get();
		if (!pScript) return;
		void* args[1] = {
			&bodyID
		};

		EntityScene* pEntityScene = pRegistry->GetUserData<EntityScene*>();
		pScript->Invoke(pEntityScene->GetEntitySceneObjectFromEntityID(entity), "OnBodyDeactivated", args);
	}

	void PhysicsSystem::OnContactAdded(uint32_t body1ID, uint32_t body2ID)
	{
		const std::pair<GloryECS::EntityRegistry*, EntityID>& pair1 = m_BodyOwners[body1ID];
		const std::pair<GloryECS::EntityRegistry*, EntityID>& pair2 = m_BodyOwners[body2ID];
		GloryECS::EntityRegistry* pRegistry1 = pair1.first;
		GloryECS::EntityRegistry* pRegistry2 = pair2.first;
		EntityID entity1 = pair1.second;
		EntityID entity2 = pair2.second;
		if (!pRegistry1->IsValid(entity1) || !pRegistry2->IsValid(entity2)) return;

		if (pRegistry1->HasComponent<ScriptedComponent>(entity1))
		{
			ScriptedComponent& scriptComponent = pRegistry1->GetComponent<ScriptedComponent>(entity1);
			Script* pScript = scriptComponent.m_Script.Get();
			if (!pScript) return;
			void* args[2] = {
				&body1ID,
				&body2ID
			};

			EntityScene* pEntityScene = pRegistry1->GetUserData<EntityScene*>();
			pScript->Invoke(pEntityScene->GetEntitySceneObjectFromEntityID(entity1), "OnContactAdded", args);
		}

		if (pRegistry2->HasComponent<ScriptedComponent>(entity2))
		{
			ScriptedComponent& scriptComponent = pRegistry2->GetComponent<ScriptedComponent>(entity2);
			Script* pScript = scriptComponent.m_Script.Get();
			if (!pScript) return;
			void* args[2] = {
				&body2ID,
				&body1ID
			};

			EntityScene* pEntityScene = pRegistry2->GetUserData<EntityScene*>();
			pScript->Invoke(pEntityScene->GetEntitySceneObjectFromEntityID(entity2), "OnContactAdded", args);
		}
	}

	void PhysicsSystem::OnContactPersisted(uint32_t body1ID, uint32_t body2ID)
	{
		const std::pair<GloryECS::EntityRegistry*, EntityID>& pair1 = m_BodyOwners[body1ID];
		const std::pair<GloryECS::EntityRegistry*, EntityID>& pair2 = m_BodyOwners[body2ID];
		GloryECS::EntityRegistry* pRegistry1 = pair1.first;
		GloryECS::EntityRegistry* pRegistry2 = pair2.first;
		EntityID entity1 = pair1.second;
		EntityID entity2 = pair2.second;
		if (!pRegistry1->IsValid(entity1) || !pRegistry2->IsValid(entity2)) return;

		if (pRegistry1->HasComponent<ScriptedComponent>(entity1))
		{
			ScriptedComponent& scriptComponent = pRegistry1->GetComponent<ScriptedComponent>(entity1);
			Script* pScript = scriptComponent.m_Script.Get();
			if (!pScript) return;
			void* args[2] = {
				&body1ID,
				&body2ID
			};

			EntityScene* pEntityScene = pRegistry1->GetUserData<EntityScene*>();
			pScript->Invoke(pEntityScene->GetEntitySceneObjectFromEntityID(entity1), "OnContactPersisted", args);
		}

		if (pRegistry2->HasComponent<ScriptedComponent>(entity2))
		{
			ScriptedComponent& scriptComponent = pRegistry2->GetComponent<ScriptedComponent>(entity2);
			Script* pScript = scriptComponent.m_Script.Get();
			if (!pScript) return;
			void* args[2] = {
				&body2ID,
				&body1ID
			};

			EntityScene* pEntityScene = pRegistry2->GetUserData<EntityScene*>();
			pScript->Invoke(pEntityScene->GetEntitySceneObjectFromEntityID(entity2), "OnContactPersisted", args);
		}
	}

	void PhysicsSystem::OnContactRemoved(uint32_t body1ID, uint32_t body2ID)
	{
		const std::pair<GloryECS::EntityRegistry*, EntityID>& pair1 = m_BodyOwners[body1ID];
		const std::pair<GloryECS::EntityRegistry*, EntityID>& pair2 = m_BodyOwners[body2ID];
		GloryECS::EntityRegistry* pRegistry1 = pair1.first;
		GloryECS::EntityRegistry* pRegistry2 = pair2.first;
		EntityID entity1 = pair1.second;
		EntityID entity2 = pair2.second;
		if (!pRegistry1->IsValid(entity1) || !pRegistry2->IsValid(entity2)) return;

		if (pRegistry1->HasComponent<ScriptedComponent>(entity1))
		{
			ScriptedComponent& scriptComponent = pRegistry1->GetComponent<ScriptedComponent>(entity1);
			Script* pScript = scriptComponent.m_Script.Get();
			if (!pScript) return;
			void* args[2] = {
				&body1ID,
				&body2ID
			};

			EntityScene* pEntityScene = pRegistry1->GetUserData<EntityScene*>();
			pScript->Invoke(pEntityScene->GetEntitySceneObjectFromEntityID(entity1), "OnContactRemoved", args);
		}

		if (pRegistry2->HasComponent<ScriptedComponent>(entity2))
		{
			ScriptedComponent& scriptComponent = pRegistry2->GetComponent<ScriptedComponent>(entity2);
			Script* pScript = scriptComponent.m_Script.Get();
			if (!pScript) return;
			void* args[2] = {
				&body2ID,
				&body1ID
			};

			EntityScene* pEntityScene = pRegistry2->GetUserData<EntityScene*>();
			pScript->Invoke(pEntityScene->GetEntitySceneObjectFromEntityID(entity2), "OnContactRemoved", args);
		}
	}

	void PhysicsSystem::SetupBody(PhysicsModule* pPhysics, GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent)
	{
		const Transform& transform = pRegistry->GetComponent<Transform>(entity);
		const Shape* pShape = pComponent.m_Shape.BaseShapePointer();

		pComponent.m_CurrentLayerIndex = pRegistry->HasComponent<LayerComponent>(entity) ? pRegistry->GetComponent<LayerComponent>(entity).m_Layer.m_LayerIndex : 0;

		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 scale;
		glm::vec3 skew;
		glm::vec4 perspective;
		if (!glm::decompose(transform.MatTransform, scale, rotation, translation, skew, perspective)) return;
		pComponent.m_BodyID = pPhysics->CreatePhysicsBody(*pShape, translation, rotation, scale, pComponent.m_BodyType, pComponent.m_CurrentLayerIndex);
		m_BodyOwners[pComponent.m_BodyID] = { pRegistry, entity };
	}
}
