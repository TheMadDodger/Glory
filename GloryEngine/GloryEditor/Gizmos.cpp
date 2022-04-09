#include "Gizmos.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <algorithm>

namespace Glory::Editor
{
	ImGuizmo::OPERATION Gizmos::m_DefaultOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE Gizmos::m_DefaultMode = ImGuizmo::LOCAL;
	std::vector<IGizmo*> Gizmos::m_pGizmos = std::vector<IGizmo*>();
	std::vector<bool> Gizmos::m_ManipulatedGizmos;

	bool Gizmos::DrawGizmo(glm::mat4* transfrom)
	{
		size_t index = m_pGizmos.size();
		m_pGizmos.push_back(new DefaultGizmo(transfrom));
		if (index >= m_ManipulatedGizmos.size()) return false;
		return m_ManipulatedGizmos[index];
	}

	void Gizmos::DrawGizmos(const glm::mat4& cameraView, const glm::mat4& cameraProjection)
	{
		m_ManipulatedGizmos.clear();
		std::for_each(m_pGizmos.begin(), m_pGizmos.end(), [&](IGizmo* pGizmo)
		{
			bool manipulated = pGizmo->OnGui(cameraView, cameraProjection);
			m_ManipulatedGizmos.push_back(manipulated);
		});
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

	bool DefaultGizmo::OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection)
	{
		return ImGuizmo::Manipulate((const float*)&cameraView, (const float*)&cameraProjection, Gizmos::m_DefaultOperation, Gizmos::m_DefaultMode, (float*)m_pTransform, NULL, NULL);//, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
	}
}