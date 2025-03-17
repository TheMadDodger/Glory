#include "JoltPhysicsModule.h"
#include "JoltPhysicsModule.h"

#include <PhysicsComponents.h>
#include <PhysicsSystem.h>
#include <Engine.h>
#include <GScene.h>
#include <SceneManager.h>
#include <Components.h>
#include <Debug.h>

#include <glm/gtx/matrix_decompose.hpp>

namespace Glory
{
	std::map<UUID, UUID> PhysicsSystem::m_CachedSceneIDs;

	PhysicsSystem::~PhysicsSystem()
	{
		OnBodyActivated_Callback = NULL;
		OnBodyDeactivated_Callback = NULL;
		OnContactAdded_Callback = NULL;
		OnContactPersisted_Callback = NULL;
		OnContactRemoved_Callback = NULL;
	}

	void PhysicsSystem::OnStart(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();

		JoltPhysicsModule* pPhysics = pEngine->GetOptionalModule<JoltPhysicsModule>();
		if (!pPhysics)
		{
			pEngine->GetDebug().LogWarning("A PhysicsBody was added to an entity but no PhysocsModule was loaded");
			return;
		}

		SetupBody(pPhysics, pRegistry, entity, pComponent);
	}

	void PhysicsSystem::OnStop(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();

		JoltPhysicsModule* pPhysics = pEngine->GetOptionalModule<JoltPhysicsModule>();
		if (!pPhysics) return;
		if (pComponent.m_BodyID == PhysicsBody::InvalidBodyID) return;
		m_CachedSceneIDs.erase(pScene->GetEntityUUID(entity));
		pPhysics->DestroyPhysicsBody(pComponent.m_BodyID);
	}

	void PhysicsSystem::OnValidate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();

		JoltPhysicsModule* pPhysics = pEngine->GetOptionalModule<JoltPhysicsModule>();
		if (!pPhysics)
		{
			pEngine->GetDebug().LogWarning("A PhysicsBody was added to an entity but no PhysicsModule was loaded");
			return;
		}

		/* If no body was created the game has not started yet */
		if(pComponent.m_BodyID == PhysicsBody::InvalidBodyID) return;

		/* Destroy the body */
		m_CachedSceneIDs.erase(pScene->GetEntityUUID(entity));
		pPhysics->DestroyPhysicsBody(pComponent.m_BodyID);

		/* Create new body */
		SetupBody(pPhysics, pRegistry, entity, pComponent);
	}

	void PhysicsSystem::OnUpdate(Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();
		Engine* pEngine = pScene->Manager()->GetEngine();

		JoltPhysicsModule* pPhysics = pEngine->GetOptionalModule<JoltPhysicsModule>();
		if (!pPhysics)
		{
			pEngine->GetDebug().LogWarning("An Entity has a PhysicsBody but no PhysocsModule was loaded");
			return;
		}

		/* An invalid body cannot be updated */
		if (pComponent.m_BodyID == PhysicsBody::InvalidBodyID) return;

		Transform& transform = pRegistry->GetComponent<Transform>(entity);
		pPhysics->PollPhysicsState(pComponent.m_BodyID, &transform.Position, &transform.Rotation);
		transform.Rotation = glm::conjugate(transform.Rotation);

		Utils::ECS::EntityView* pEntityView = pRegistry->GetEntityView(entity);
		Entity parent = pRegistry->GetUserData<GScene*>()->GetEntityByEntityID(pEntityView->Parent());
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

		pRegistry->SetEntityDirty(entity);
	}

	void PhysicsSystem::OnBodyActivated(JoltPhysicsModule* pPhysics, uint32_t bodyID)
	{
		const UUID entityUUID = pPhysics->GetBodyUserData(bodyID);
		auto iter = m_CachedSceneIDs.find(entityUUID);
		if (iter == m_CachedSceneIDs.end()) return;
		GScene* pScene = pPhysics->GetEngine()->GetSceneManager()->GetOpenScene(iter->second);
		if (!pScene) return;
		if (!Instance()->OnBodyActivated_Callback) return;
		Instance()->OnBodyActivated_Callback(pPhysics->GetEngine(), pScene->GetUUID(), entityUUID);
	}

	void PhysicsSystem::OnBodyDeactivated(JoltPhysicsModule* pPhysics, uint32_t bodyID)
	{
		const UUID entityUUID = pPhysics->GetBodyUserData(bodyID);
		auto iter = m_CachedSceneIDs.find(entityUUID);
		if (iter == m_CachedSceneIDs.end()) return;
		GScene* pScene = pPhysics->GetEngine()->GetSceneManager()->GetOpenScene(iter->second);
		if (!pScene) return;
		if (!Instance()->OnBodyDeactivated_Callback) return;
		Instance()->OnBodyDeactivated_Callback(pPhysics->GetEngine(), pScene->GetUUID(), entityUUID);
	}

	void PhysicsSystem::OnContactAdded(JoltPhysicsModule* pPhysics, uint32_t body1ID, uint32_t body2ID)
	{
		const UUID entity1UUID = pPhysics->GetBodyUserData(body1ID);
		const UUID entity2UUID = pPhysics->GetBodyUserData(body2ID);
		auto iter1 = m_CachedSceneIDs.find(entity1UUID);
		auto iter2 = m_CachedSceneIDs.find(entity2UUID);
		if (iter1 == m_CachedSceneIDs.end()) return;
		if (iter2 == m_CachedSceneIDs.end()) return;
		GScene* pScene1 = pPhysics->GetEngine()->GetSceneManager()->GetOpenScene(iter1->second);
		GScene* pScene2 = pPhysics->GetEngine()->GetSceneManager()->GetOpenScene(iter2->second);
		if (!pScene1 || !pScene2) return;
		if (!Instance()->OnContactAdded_Callback) return;
		Instance()->OnContactAdded_Callback(pPhysics->GetEngine(), pScene1->GetUUID(), entity1UUID, pScene2->GetUUID(), entity2UUID);
	}

	void PhysicsSystem::OnContactPersisted(JoltPhysicsModule* pPhysics, uint32_t body1ID, uint32_t body2ID)
	{
		const UUID entity1UUID = pPhysics->GetBodyUserData(body1ID);
		const UUID entity2UUID = pPhysics->GetBodyUserData(body2ID);
		auto iter1 = m_CachedSceneIDs.find(entity1UUID);
		auto iter2 = m_CachedSceneIDs.find(entity2UUID);
		if (iter1 == m_CachedSceneIDs.end()) return;
		if (iter2 == m_CachedSceneIDs.end()) return;
		GScene* pScene1 = pPhysics->GetEngine()->GetSceneManager()->GetOpenScene(iter1->second);
		GScene* pScene2 = pPhysics->GetEngine()->GetSceneManager()->GetOpenScene(iter2->second);
		if (!pScene1 || !pScene2) return;
		if (!Instance()->OnContactPersisted_Callback) return;
		Instance()->OnContactPersisted_Callback(pPhysics->GetEngine(), pScene1->GetUUID(), entity1UUID, pScene2->GetUUID(), entity2UUID);
	}

	void PhysicsSystem::OnContactRemoved(JoltPhysicsModule* pPhysics, uint32_t body1ID, uint32_t body2ID)
	{
		const UUID entity1UUID = pPhysics->GetBodyUserData(body1ID);
		const UUID entity2UUID = pPhysics->GetBodyUserData(body2ID);
		auto iter1 = m_CachedSceneIDs.find(entity1UUID);
		auto iter2 = m_CachedSceneIDs.find(entity2UUID);
		if (iter1 == m_CachedSceneIDs.end()) return;
		if (iter2 == m_CachedSceneIDs.end()) return;
		GScene* pScene1 = pPhysics->GetEngine()->GetSceneManager()->GetOpenScene(iter1->second);
		GScene* pScene2 = pPhysics->GetEngine()->GetSceneManager()->GetOpenScene(iter2->second);
		if (!pScene1 || !pScene2) return;
		if (!Instance()->OnContactRemoved_Callback) return;
		Instance()->OnContactRemoved_Callback(pPhysics->GetEngine(), pScene1->GetUUID(), entity1UUID, pScene2->GetUUID(), entity2UUID);
	}

	void PhysicsSystem::AddToSceneIDsCache(UUID entityUUID, UUID sceneID)
	{
		m_CachedSceneIDs.emplace(entityUUID, sceneID);
	}

	void PhysicsSystem::RemoveFromSceneIDsCache(UUID entityUUID)
	{
		m_CachedSceneIDs.erase(entityUUID);
	}

	PhysicsSystem* PhysicsSystem::Instance()
	{
		static PhysicsSystem Inst;
		return &Inst;
	}

	void PhysicsSystem::SetupBody(JoltPhysicsModule* pPhysics, Utils::ECS::EntityRegistry* pRegistry, Utils::ECS::EntityID entity, PhysicsBody& pComponent)
	{
		GScene* pScene = pRegistry->GetUserData<GScene*>();

		pComponent.m_BodyID = PhysicsBody::InvalidBodyID;

		const Transform& transform = pRegistry->GetComponent<Transform>(entity);
		const Shape* pShape = pComponent.m_Shape.BaseShapePointer();
		if (pShape->m_ShapeType == ShapeType::None)
		{
			pPhysics->GetEngine()->GetDebug().LogWarning("PhysicsBody does not have a shape!");
			return;
		}

		pComponent.m_CurrentLayerIndex = pRegistry->HasComponent<LayerComponent>(entity) ? pRegistry->GetComponent<LayerComponent>(entity).m_Layer.m_LayerIndex : 0;

		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 scale;
		glm::vec3 skew;
		glm::vec4 perspective;
		if (!glm::decompose(transform.MatTransform, scale, rotation, translation, skew, perspective)) return;
		pComponent.m_BodyID = pPhysics->CreatePhysicsBody(*pShape, translation, rotation, scale, pComponent.m_BodyType, pComponent.m_CurrentLayerIndex);
		const UUID entityUUID = pScene->GetEntityUUID(entity);
		pPhysics->SetBodyUserData(pComponent.m_BodyID, entityUUID);
		m_CachedSceneIDs.emplace(entityUUID, pScene->GetUUID());
	}
}
