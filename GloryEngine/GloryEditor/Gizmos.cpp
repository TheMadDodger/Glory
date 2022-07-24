#include "Gizmos.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <algorithm>

namespace Glory::Editor
{
	std::vector<IGizmo*> Gizmos::m_pGizmos;
	std::vector<bool> Gizmos::m_ManipulatedGizmos;

	ImGuizmo::OPERATION Gizmos::m_DefaultOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE Gizmos::m_DefaultMode = ImGuizmo::LOCAL;

	GLORY_EDITOR_API bool Gizmos::DrawGizmo(glm::mat4* transfrom)
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

	GLORY_EDITOR_API Gizmos::Gizmos() {}
	GLORY_EDITOR_API Gizmos::~Gizmos() {}

	GLORY_EDITOR_API IGizmo::IGizmo() {}
	GLORY_EDITOR_API IGizmo::~IGizmo() {}

	GLORY_EDITOR_API DefaultGizmo::DefaultGizmo(glm::mat4* pTransform) : m_pTransform(pTransform)
	{
	}

	GLORY_EDITOR_API DefaultGizmo::~DefaultGizmo()
	{
		m_pTransform = nullptr;
	}

	GLORY_EDITOR_API bool DefaultGizmo::OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection)
	{
		return ImGuizmo::Manipulate((const float*)&cameraView, (const float*)&cameraProjection, Gizmos::m_DefaultOperation, Gizmos::m_DefaultMode, (float*)m_pTransform, NULL, NULL);//, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
	}
}