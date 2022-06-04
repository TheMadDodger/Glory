#include "ScriptedSystem.h"
#include <AssetReferencePropertyTemplate.h>
#include <SerializedPropertyManager.h>

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
		if (!pComponent.m_pScript) return;
		pComponent.m_pScript->Invoke(pComponent.m_pScript, "Update()");
	}

	void ScriptedSystem::OnDraw(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		if (pComponent.m_pScript) return;
		//pComponent.m_pScript->Invoke("Draw");
	}

	void ScriptedSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, ScriptedComponent& pComponent)
	{
		properties.push_back(SerializedPropertyManager::GetProperty< AssetReferencePropertyTemplate<Script>>(uuid, "Script", &pComponent.m_pScript, 0));
	}

	std::string ScriptedSystem::Name()
	{
		return "Script";
	}
}
