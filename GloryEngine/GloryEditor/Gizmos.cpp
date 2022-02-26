#include "Gizmos.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <algorithm>

namespace Glory::Editor
{
	ImGuizmo::OPERATION Gizmos::m_DefaultOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE Gizmos::m_DefaultMode = ImGuizmo::LOCAL;
	std::vector<IGizmo*> Gizmos::m_pGizmos = std::vector<IGizmo*>();

	void Gizmos::DrawGizmo(glm::mat4* transfrom)
	{
		m_pGizmos.push_back(new DefaultGizmo(transfrom));
	}

	void Gizmos::DrawGizmos(const glm::mat4& cameraView, const glm::mat4& cameraProjection)
	{
		std::for_each(m_pGizmos.begin(), m_pGizmos.end(), [&](IGizmo* pGizmo) { pGizmo->OnGui(cameraView, cameraProjection); });
		Clear();
	}

	void Gizmos::Clear()
	{
		std::for_each(m_pGizmos.begin(), m_pGizmos.end(), [](IGizmo* pGizmo) { delete pGizmo; });
		m_pGizmos.clear();
	}

	Gizmos::Gizmos() {}
	Gizmos::~Gizmos() {}

	IGizmo::IGizmo() {}
	IGizmo::~IGizmo() {}

	DefaultGizmo::DefaultGizmo(glm::mat4* pTransform) : m_pTransform(pTransform)
	{
	}

	DefaultGizmo::~DefaultGizmo()
	{
		m_pTransform = nullptr;
	}

	void DefaultGizmo::OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection)
	{
		ImGuizmo::Manipulate(&cameraView[0][0], &cameraProjection[0][0], Gizmos::m_DefaultOperation, Gizmos::m_DefaultMode, (float*)m_pTransform, NULL, NULL);//, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
	}
}