#include "TransformEditor.h"
#include "Undo.h"
#include "GizmoAction.h"

#include <Engine.h>
#include <PhysicsModule.h>
#include <glm/gtx/quaternion.hpp>
#include <TypeData.h>
#include <CharacterManager.h>
#include <EditorUI.h>
#include <TransformSystem.h>
#include <glm/gtx/matrix_decompose.hpp>

namespace Glory::Editor
{
	TransformEditor::TransformEditor() : m_pGizmo(nullptr)
	{
	}

	TransformEditor::~TransformEditor()
	{
		m_pGizmo->OnManualManipulate = NULL;
		Gizmos::FreeGizmo(m_pTarget->GetUUID());
	}

	void TransformEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
		Transform& transform = GetTargetComponent();

		m_pGizmo = Gizmos::GetGizmo<DefaultGizmo>(m_pTarget->GetUUID(), transform.MatTransform);
		m_pGizmo->OnManualManipulate = [&](const glm::mat4& newTransform) {
			UpdateTransform(newTransform);
		};
	}

	bool TransformEditor::OnGUI()
	{
		glm::mat4 oldTransform, newTransform;
		bool wasManipulated = m_pGizmo->WasManipulated(oldTransform, newTransform);
		const bool isManipulating = m_pGizmo->IsManipulating();

		bool change = false;

		const EntityID entity = m_pComponentObject->EntityID();
		const TypeData* pTypeData = Transform::GetTypeData();
		Transform& transform = GetTargetComponent();

		if (!wasManipulated && !isManipulating && transform.MatTransform != newTransform)
		{
			m_pGizmo->UpdateTransform(transform.MatTransform);
		}

		glm::vec3 pos = transform.Position;
		glm::quat rotation = transform.Rotation;
		glm::vec3 scale = transform.Scale;

		Undo::StartRecord("Property Change", m_pComponentObject->GetUUID(), true);
		if (EditorUI::InputFloat3("Position", &pos))
		{
			ValueChangeAction* pAction = new ValueChangeAction(pTypeData, "Position");
			pAction->SetOldValue(&transform.Position);
			pAction->SetNewValue(&pos);
			Undo::AddAction(pAction);

			change = true;
		}

		glm::vec3 euler = glm::eulerAngles(rotation) / 3.141592f * 180.0f;
		if (PropertyDrawerTemplate<glm::quat>().OnGUI("Rotation", &rotation, 0))
		{
			ValueChangeAction* pAction = new ValueChangeAction(pTypeData, "Rotation");
			pAction->SetOldValue(&transform.Rotation);
			pAction->SetNewValue(&rotation);
			Undo::AddAction(pAction);

			change = true;
		}

		if (EditorUI::InputFloat3("Scale", &scale))
		{
			ValueChangeAction* pAction = new ValueChangeAction(pTypeData, "Scale");
			pAction->SetOldValue(&transform.Scale);
			pAction->SetNewValue(&scale);
			Undo::AddAction(pAction);

			change = true;
		}
		Undo::StopRecord();

		if (change)
		{
			transform.Position = pos;
			transform.Rotation = rotation;
			transform.Scale = scale;
			TransformSystem::OnUpdate(m_pComponentObject->GetRegistry(), entity, transform);
			UpdatePhysics();
		}

		if (change)
		{
			Validate();
			m_pGizmo->UpdateTransform(transform.MatTransform);
			wasManipulated = false;
		}

		if (isManipulating)
		{
			UpdateTransform(newTransform);
			UpdatePhysics();
		}

		if (wasManipulated)
		{
			Undo::StartRecord("Transform");
			Undo::AddAction(new GizmoAction(m_pTarget->GetUUID(), oldTransform, newTransform));
			Undo::StopRecord();
			UpdateTransform(newTransform);
			return change;
		}

		return change;
	}

	std::string TransformEditor::Name()
	{
		return "Transform";
	}

	void TransformEditor::UpdateTransform(const glm::mat4 newTransform)
	{
		Transform& transform = GetTargetComponent();

		glm::mat4 localTransform = newTransform;

		if (transform.Parent.IsValid())
		{
			Transform& parentTransform = transform.Parent.GetComponent<Transform>();
			localTransform = glm::inverse(parentTransform.MatTransform) * localTransform;
		}
		glm::vec3 right(localTransform[0][0], localTransform[0][1], localTransform[0][2]);
		glm::vec3 up(localTransform[1][0], localTransform[1][1], localTransform[1][2]);
		glm::vec3 forward(localTransform[2][0], localTransform[2][1], localTransform[2][2]);
		glm::vec3 position(localTransform[3][0], localTransform[3][1], localTransform[3][2]);

		glm::vec3 scale;
		scale.x = glm::length(right);
		scale.y = glm::length(up);
		scale.z = glm::length(forward);

		right /= scale.x;
		up /= scale.y;
		forward /= scale.z;

		transform.Position = position;
		transform.Rotation = glm::conjugate(glm::quatLookAt(-forward, up));
		transform.Scale = scale;
	}

	void TransformEditor::UpdatePhysics()
	{
		PhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetMainModule<PhysicsModule>();
		if (!pPhysics) return;

		const EntityID entity = m_pComponentObject->EntityID();
		EntityRegistry* pRegistry = m_pComponentObject->GetRegistry();
		if (pRegistry->HasComponent<PhysicsBody>(entity))
		{
			Transform& transform = pRegistry->GetComponent<Transform>(entity);
			PhysicsBody& physicsBody = pRegistry->GetComponent<PhysicsBody>(entity);
			PhysicsModule* pPhysics = Game::GetGame().GetEngine()->GetMainModule<PhysicsModule>();
			if (pPhysics && pPhysics->IsValidBody(physicsBody.m_BodyID))
			{
				glm::quat rotation;
				glm::vec3 translation, scale, skew;
				glm::vec4 perspective;
				if (glm::decompose(transform.MatTransform, scale, rotation, translation, skew, perspective))
				{
					pPhysics->SetBodyPosition(physicsBody.m_BodyID, translation);
					pPhysics->SetBodyRotation(physicsBody.m_BodyID, rotation);
					pPhysics->SetBodyScale(physicsBody.m_BodyID, scale);
					pPhysics->SetBodyLinearAndAngularVelocity(physicsBody.m_BodyID, {}, {});
				}
			}
		}

		CharacterManager* pCharacters = pPhysics->GetCharacterManager();
		if (!pCharacters) return;

		if (pRegistry->HasComponent<CharacterController>(entity))
		{
			Transform& transform = pRegistry->GetComponent<Transform>(entity);
			CharacterController& characterController = pRegistry->GetComponent<CharacterController>(entity);
			if (pPhysics && characterController.m_CharacterID)
			{
				glm::quat rotation;
				glm::vec3 translation, scale, skew;
				glm::vec4 perspective;
				if (glm::decompose(transform.MatTransform, scale, rotation, translation, skew, perspective))
				{
					pCharacters->SetPosition(characterController.m_CharacterID, translation);
					pCharacters->SetRotation(characterController.m_CharacterID, rotation);
					pCharacters->SetLinearAndAngularVelocity(characterController.m_CharacterID, {}, {});
				}
			}
		}
	}
}