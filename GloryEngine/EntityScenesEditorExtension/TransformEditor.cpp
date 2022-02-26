#include "TransformEditor.h"
#include <Gizmos.h>
#include <glm/gtx/quaternion.hpp>

namespace Glory::Editor
{
	TransformEditor::TransformEditor() : m_Transform(glm::mat4()), m_LastTransform(glm::mat4())
	{
	}

	TransformEditor::~TransformEditor()
	{
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
	}

	void TransformEditor::OnGUI()
	{
		UpdateTransform();
		EntityComponentEditor::OnGUI();
		Gizmos::DrawGizmo(&m_Transform);
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
		transform.Rotation = glm::conjugate(glm::toQuat(glm::lookAt(glm::vec3(), forward, up)));
		transform.Scale = scale;

		m_LastTransform = m_Transform;
	}
}