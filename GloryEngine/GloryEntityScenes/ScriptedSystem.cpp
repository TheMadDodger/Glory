#include "ScriptedSystem.h"
#include <AssetReferencePropertyTemplate.h>
#include <SerializedPropertyManager.h>
#include <AssetManager.h>

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
		if (!pComponent.m_Script) return;
		Script* pScript = AssetManager::GetOrLoadAsset<Script>(pComponent.m_Script);
		if (pScript == nullptr) return;
		pScript->Invoke(pScript, "Update()");
	}

	void ScriptedSystem::OnDraw(Registry* pRegistry, EntityID entity, ScriptedComponent& pComponent)
	{
		if (pComponent.m_Script) return;
		//pComponent.m_pScript->Invoke("Draw");
	}

	void ScriptedSystem::OnAcquireSerializedProperties(UUID uuid, std::vector<SerializedProperty*>& properties, ScriptedComponent& pComponent)
	{
		properties.push_back(SerializedPropertyManager::GetProperty< AssetReferencePropertyTemplate<Script>>(uuid, "Script", &pComponent.m_Script, 0));
	}

	std::string ScriptedSystem::Name()
	{
		return "Script";
	}
}
