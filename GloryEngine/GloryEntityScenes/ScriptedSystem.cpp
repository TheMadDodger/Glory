#include "ScriptedSystem.h"

namespace Glory
{
	void ScriptedSystem::OnComponentAdded(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
	}

	void ScriptedSystem::OnComponentRemoved(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
	}

	void ScriptedSystem::OnUpdate(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		if (pComponent.m_pScript) return;
		//pComponent.m_pScript->Invoke("Update");
	}

	void ScriptedSystem::OnDraw(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		if (pComponent.m_pScript) return;
		//pComponent.m_pScript->Invoke("Draw");
	}

	void ScriptedSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, ScriptedComponent& pComponent)
	{
		if (pComponent.m_pScript) return;
		//pComponent.m_pScript->Invoke("Update");
	}

	std::string ScriptedSystem::Name()
	{
		return "Script";
	}
}
