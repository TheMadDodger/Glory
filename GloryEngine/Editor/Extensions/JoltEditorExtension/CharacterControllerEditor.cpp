#include "CharacterControllerEditor.h"
#include "Undo.h"
#include "GizmoAction.h"

#include <EditorApplication.h>
#include <Engine.h>
#include <RendererModule.h>
#include <glm/gtx/quaternion.hpp>
#include <TypeData.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <Components.h>

namespace Glory::Editor
{
	CharacterControllerEditor::CharacterControllerEditor()
	{
	}

	CharacterControllerEditor::~CharacterControllerEditor()
	{
	}

	void CharacterControllerEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
	}

	bool CharacterControllerEditor::OnGUI()
	{
		Transform& transform = m_pComponentObject->GetRegistry()->GetComponent<Transform>(m_pComponentObject->EntityID());
		CharacterController& body = GetTargetComponent();
		EditorApplication::GetInstance()->GetEngine()->GetMainModule<RendererModule>()->DrawLineShape(transform.MatTransform, {}, body.m_Shape, {1, 0, 0, 1});
		return EntityComponentEditor::OnGUI();
	}

	std::string CharacterControllerEditor::Name()
	{
		return "Character Controller";
	}
}