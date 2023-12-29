#include "MonoScriptComponentEditor.h"

#include <AssetManager.h>

namespace Glory::Editor
{
	MonoScriptComponentEditor::MonoScriptComponentEditor() : m_pScript(nullptr)
	{
	}

	MonoScriptComponentEditor::~MonoScriptComponentEditor()
	{
	}

	bool DrawProperty(const ScriptProperty& scriptProperty, YAML::Node& node, uint32_t flags)
	{
		PropertyDrawer::SetCurrentPropertyPath(scriptProperty.m_Name);
		const uint32_t typeHash = scriptProperty.m_TypeHash;
		const uint32_t elementTypeHash = scriptProperty.m_ElementTypeHash;

		PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(typeHash);

		if (pPropertyDrawer)
			return pPropertyDrawer->Draw(scriptProperty.m_Name, node[scriptProperty.m_Name], elementTypeHash, flags);

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), scriptProperty.m_Name);
		return false;
	}

	void MonoScriptComponentEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
		MonoScriptComponent& scriptComponent = GetTargetComponent();
		if (!scriptComponent.m_Script.AssetUUID()) return;
		m_pScript = AssetManager::GetAssetImmediate<MonoScript>(scriptComponent.m_Script.AssetUUID());
		if (!m_pScript)
		{
			scriptComponent.m_Script.SetUUID(0);
			return;
		}

		m_pScript->LoadScriptProperties(scriptComponent.m_ScriptProperties, scriptComponent.m_ScriptData);
	}

	bool MonoScriptComponentEditor::OnGUI()
	{
		bool change = EntityComponentEditor::OnGUI();
		MonoScriptComponent& scriptComponent = GetTargetComponent();
		if (m_pScript == nullptr && scriptComponent.m_Script.AssetUUID())
		{
			Initialize();
			return change;
		}

		if (m_pScript != nullptr && m_pScript->GetUUID() != scriptComponent.m_Script.AssetUUID())
		{
			Initialize();
			return change;
		}

		if (!m_pScript) return change;

		Undo::StartRecord("Property Change", m_pComponentObject->GetUUID(), true);
		bool changedScriptProp = false;
		for (size_t i = 0; i < scriptComponent.m_ScriptProperties.size(); i++)
		{
			const ScriptProperty* scriptProperty = &scriptComponent.m_ScriptProperties[i];
			if (std::string_view{ scriptProperty->m_Name }.empty()) continue;
			changedScriptProp |= DrawProperty(*scriptProperty, scriptComponent.m_ScriptData, 0);
		}

		if (changedScriptProp) Validate();
		Undo::StopRecord();
		return change || changedScriptProp;
	}

	std::string MonoScriptComponentEditor::Name()
	{
		return "Mono Script";
	}
}
