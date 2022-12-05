#include "Gizmos.h"
#include "Undo.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <algorithm>

namespace Glory::Editor
{
	std::map<UUID, IGizmo*> Gizmos::m_pGizmos;
	//std::vector<bool> Gizmos::m_ManipulatedGizmos;

	ImGuizmo::OPERATION Gizmos::m_DefaultOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE Gizmos::m_DefaultMode = ImGuizmo::LOCAL;

	GizmosToolChain* Gizmos::m_pToolChain = nullptr;

	//GLORY_EDITOR_API bool Gizmos::DrawGizmo(glm::mat4* transfrom)
	//{
	//	size_t index = m_pGizmos.size();
	//	m_pGizmos.push_back(new DefaultGizmo(transfrom));
	//	if (index >= m_ManipulatedGizmos.size()) return false;
	//	return m_ManipulatedGizmos[index];
	//}

	void Gizmos::AddGizmo(UUID uuid, IGizmo* pGizmo)
	{
		m_pGizmos.emplace(uuid, pGizmo);
	}

	GLORY_EDITOR_API IGizmo* Gizmos::FindGizmo(UUID uuid)
	{
		if (m_pGizmos.find(uuid) == m_pGizmos.end()) return nullptr;
		return m_pGizmos[uuid];
	}

	void Gizmos::FreeGizmo(UUID uuid)
	{
		if (m_pGizmos.find(uuid) == m_pGizmos.end()) return;
		delete m_pGizmos[uuid];
		m_pGizmos.erase(uuid);
	}

	void Gizmos::DrawGizmos(const glm::mat4& cameraView, const glm::mat4& cameraProjection)
	{
		for (auto it = m_pGizmos.begin(); it != m_pGizmos.end(); it++)
		{
			IGizmo* pGizmo = it->second;
			pGizmo->OnGui(cameraView, cameraProjection);
		}
	}

	void Gizmos::Clear()
	{
		for (auto it = m_pGizmos.begin(); it != m_pGizmos.end(); it++)
		{
			IGizmo* pGizmo = it->second;
			delete pGizmo;
		}
		m_pGizmos.clear();
	}

	void Gizmos::Initialize()
	{
		m_pToolChain = new GizmosToolChain();
	}

	void Gizmos::Cleanup()
	{
		delete m_pToolChain;
		m_pToolChain = nullptr;
	}

	GLORY_EDITOR_API Gizmos::Gizmos() {}
	GLORY_EDITOR_API Gizmos::~Gizmos() {}

	GLORY_EDITOR_API IGizmo::IGizmo() {}
	GLORY_EDITOR_API IGizmo::~IGizmo() {}

	GLORY_EDITOR_API DefaultGizmo::DefaultGizmo(glm::mat4 transform) : m_Transform(transform), m_OldTransform(transform), m_IsManipulating(false), m_WasManipulated(false)
	{
	}

	GLORY_EDITOR_API DefaultGizmo::~DefaultGizmo()
	{
	}

	void DefaultGizmo::OnGui(const glm::mat4& cameraView, const glm::mat4& cameraProjection)
	{
		m_WasManipulated = false;
		bool manipulating = ImGuizmo::Manipulate((const float*)&cameraView, (const float*)&cameraProjection, Gizmos::m_DefaultOperation, Gizmos::m_DefaultMode, (float*)&m_Transform, NULL, NULL);//, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
		if (manipulating && !m_IsManipulating)
		{
			m_IsManipulating = true;
		}
		else if (!manipulating && m_IsManipulating && !ImGuizmo::IsUsing())
		{
			m_IsManipulating = false;
			m_WasManipulated = true;
		}
	}

	bool DefaultGizmo::WasManipulated(glm::mat4& oldTransform, glm::mat4& newTransform)
	{
		newTransform = m_Transform;
		oldTransform = m_OldTransform;
		if (m_WasManipulated) m_OldTransform = m_Transform;
		return m_WasManipulated;
	}

	void DefaultGizmo::ManualManipulate(const glm::mat4& newTransform)
	{
		m_Transform = newTransform;
		m_OldTransform = m_Transform;
		if (OnManualManipulate != NULL) OnManualManipulate(m_Transform);
	}

	void DefaultGizmo::UpdateTransform(const glm::mat4& newTransform)
	{
		m_Transform = newTransform;
		m_OldTransform = m_Transform;
	}
}