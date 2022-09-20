#include "TransformEditor.h"
#include "Undo.h"
#include "GizmoAction.h"
#include <glm/gtx/quaternion.hpp>
#include <sstream>

namespace Glory::Editor
{
	TransformEditor::TransformEditor() : m_Transform(glm::mat4()), m_LastTransform(glm::mat4()), m_pGizmo(nullptr)
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

		glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), transform.Scale);
		glm::mat4 rotation = glm::mat4_cast(transform.Rotation);
		//glm::mat4 rotation = glm::eulerAngleXYX(90.0f, 90.0f, 0.0f);
		glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), transform.Position);
		m_Transform = translation * rotation * scale;
		m_LastTransform = m_Transform;
		m_pGizmo = Gizmos::GetGizmo<DefaultGizmo>(m_pTarget->GetUUID(), m_Transform);

		m_pGizmo->OnManualManipulate = [&](const glm::mat4& newTransform)
		{
			m_Transform = newTransform;
			UpdateTransform();
		};
	}

	bool TransformEditor::OnGUI()
	{
		bool change = EntityComponentEditor::OnGUI();
		UpdateTransform();
		if (change) m_pGizmo->UpdateTransform(m_Transform);
		glm::mat4 oldTransform;
		if (!m_pGizmo->WasManipulated(oldTransform, m_Transform)) return change;

		Undo::StartRecord("Transform");
		Undo::AddAction(new GizmoAction(m_pTarget->GetUUID(), oldTransform, m_Transform));
		Undo::StopRecord();
		return true;
	}

	std::string TransformEditor::Name()
	{
		return "Transform";
	}

	void TransformEditor::UpdateTransform()
	{
		Transform& transform = GetTargetComponent();

		if (m_LastTransform == m_Transform)
		{
			glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), transform.Scale);
			glm::mat4 rotation = glm::mat4_cast(transform.Rotation);
			//glm::mat4 rotation = glm::eulerAngleXYX(90.0f, 90.0f, 0.0f);
			glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), transform.Position);
			m_Transform = translation * rotation * scale;
			m_LastTransform = m_Transform;
			return;
		}

		glm::vec3 right(m_Transform[0][0], m_Transform[0][1], m_Transform[0][2]);
		glm::vec3 up(m_Transform[1][0], m_Transform[1][1], m_Transform[1][2]);
		glm::vec3 forward(m_Transform[2][0], m_Transform[2][1], m_Transform[2][2]);
		glm::vec3 position(m_Transform[3][0], m_Transform[3][1], m_Transform[3][2]);

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

		m_LastTransform = m_Transform;
	}

	void TransformEditor::PrintData(Transform& transform)
	{
		Debug::LogInfo("Transform Data Start");
		std::stringstream stream;
		stream << "Position: " << transform.Position.x << ", " << transform.Position.y << ", " << transform.Position.z;
		Debug::LogInfo(stream.str());
		stream = std::stringstream();
		stream << "Rotation: " << transform.Rotation.x << ", " << transform.Rotation.y << ", " << transform.Rotation.z << ", " << transform.Rotation.w;
		Debug::LogInfo(stream.str());
		stream = std::stringstream();
		stream << "Scale: " << transform.Scale.x << ", " << transform.Scale.y << ", " << transform.Scale.z;
		Debug::LogInfo(stream.str());
		Debug::LogInfo("Transform Data End");
	}
}