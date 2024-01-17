#include "ScriptedComponentEditor.h"
#include "Undo.h"
#include "GizmoAction.h"
#include "EditorApplication.h"

#include <glm/gtx/quaternion.hpp>
#include <sstream>
#include <TypeData.h>
#include <AssetManager.h>
#include <Script.h>

namespace Glory::Editor
{
	ScriptedComponentEditor::ScriptedComponentEditor() : m_pScript(nullptr)
	{
	}

	ScriptedComponentEditor::~ScriptedComponentEditor()
	{
	}

	void ScriptedComponentEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
		ScriptedComponent& scriptComponent = GetTargetComponent();
		if (!scriptComponent.m_Script.AssetUUID()) return;
		m_pScript = EditorApplication::GetInstance()->GetEngine()->GetAssetManager().GetAssetImmediate<Script>(scriptComponent.m_Script.AssetUUID());
		if (!m_pScript)
		{
			scriptComponent.m_Script.SetUUID(0);
			return;
		}

		m_pScript->LoadScriptProperties(scriptComponent.m_ScriptProperties, scriptComponent.m_ScriptData);
	}

	bool ScriptedComponentEditor::OnGUI()
	{
		bool change = EntityComponentEditor::OnGUI();
		ScriptedComponent& scriptComponent = GetTargetComponent();
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
			if (std::string_view{scriptProperty->m_Name}.empty()) continue;
			changedScriptProp |= PropertyDrawer::DrawProperty(*scriptProperty, scriptComponent.m_ScriptData, 0);
		}

		if (changedScriptProp) Validate();
		Undo::StopRecord();
		return change || changedScriptProp;
	}

	std::string ScriptedComponentEditor::Name()
	{
		return "Script";
	}
}