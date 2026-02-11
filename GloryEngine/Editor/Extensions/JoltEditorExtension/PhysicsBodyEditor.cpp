#include "PhysicsBodyEditor.h"
#include "Undo.h"
#include "GizmoAction.h"

#include <EditorApplication.h>
#include <Engine.h>
#include <Renderer.h>
#include <glm/gtx/quaternion.hpp>
#include <TypeData.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <Components.h>

namespace Glory::Editor
{
	PhysicsBodyEditor::PhysicsBodyEditor()
	{
	}

	PhysicsBodyEditor::~PhysicsBodyEditor()
	{
	}

	void PhysicsBodyEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
	}

	bool PhysicsBodyEditor::OnGUI()
	{
		Transform& transform = m_pComponentObject->GetRegistry()->GetComponent<Transform>(m_pComponentObject->EntityID());
		PhysicsBody& body = GetTargetComponent();
		EditorApplication::GetInstance()->GetEngine()->ActiveRenderer()->DrawLineShape(transform.MatTransform, {}, body.m_Shape, {1, 0, 0, 1});
		return EntityComponentEditor::OnGUI();
	}

	std::string PhysicsBodyEditor::Name()
	{
		return "Physics Body";
	}
}