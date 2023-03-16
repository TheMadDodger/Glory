#include "PhysicsSystem.h"

namespace Glory
{
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
		/* TODO: Convert results to local space before assigning */
		pPhysics->PollPhysicsState(pComponent.m_BodyID, &transform.Position, &transform.Rotation);
	}

	void PhysicsSystem::SetupBody(PhysicsModule* pPhysics, GloryECS::EntityRegistry* pRegistry, EntityID entity, PhysicsBody& pComponent)
	{
		const Transform& transform = pRegistry->GetComponent<Transform>(entity);

		/* TODO: Calculate world rotation from transform matrix */
		pComponent.m_BodyID = pPhysics->CreatePhysicsBody(Sphere(0.5f), transform.MatTransform[3], glm::identity<glm::quat>(), pComponent.m_BodyType);
	}
}
