#include "MonoScriptComponentEditor.h"
#include "MonoEditorExtension.h"

#include <AssetManager.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
	MonoScriptComponentEditor::MonoScriptComponentEditor()
	{
	}

	MonoScriptComponentEditor::~MonoScriptComponentEditor()
	{
	}

	bool DrawProperty(const ScriptProperty& scriptProperty, std::vector<char>& data, uint32_t flags)
	{
		PropertyDrawer::SetCurrentPropertyPath(scriptProperty.m_Name);
		const uint32_t typeHash = scriptProperty.m_TypeHash;
		const uint32_t elementTypeHash = scriptProperty.m_ElementTypeHash;

		PropertyDrawer* pPropertyDrawer = PropertyDrawer::GetPropertyDrawer(typeHash);

		bool change = false;

		if (pPropertyDrawer)
		{
			void* offsettedData = &data[scriptProperty.m_RelativeOffset];
			change = pPropertyDrawer->Draw(scriptProperty.m_Name, offsettedData, elementTypeHash, flags);
		}
		else
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), scriptProperty.m_Name);
		}

		PropertyDrawer::SetCurrentPropertyPath("");
		return change;
	}

	void MonoScriptComponentEditor::Initialize()
	{
		m_LastCompilationVersion = MonoEditorExtension::CompilationVersion();
		EntityComponentEditor::Initialize();
		MonoScriptComponent& scriptComponent = GetTargetComponent();
		if (!scriptComponent.m_Script.AssetUUID()) return;
		MonoScript* pScript = EditorApplication::GetInstance()->GetEngine()->GetAssetManager().GetAssetImmediate<MonoScript>(scriptComponent.m_Script.AssetUUID());
		if (!pScript)
		{
			scriptComponent.m_Script.SetUUID(0);
			return;
		}

		scriptComponent.m_ScriptProperties.clear();
		pScript->LoadScriptProperties();
		pScript->GetScriptProperties(scriptComponent.m_ScriptProperties);
		pScript->ReadDefaults(scriptComponent.m_ScriptData.m_Buffer);
	}

	bool MonoScriptComponentEditor::OnGUI()
	{
		const size_t compilationVersion = MonoEditorExtension::CompilationVersion();

		bool change = EntityComponentEditor::OnGUI();
		MonoScriptComponent& scriptComponent = GetTargetComponent();
		if (change && scriptComponent.m_Script.AssetUUID() || m_LastCompilationVersion != compilationVersion)
		{
			Initialize();
			return change;
		}
		MonoScript* pScript = EditorApplication::GetInstance()->GetEngine()->GetAssetManager().GetAssetImmediate<MonoScript>(scriptComponent.m_Script.AssetUUID());
		if (!pScript) return change;

		Undo::StartRecord("Property Change", m_pComponentObject->GetUUID(), true);
		bool changedScriptProp = false;
		for (size_t i = 0; i < scriptComponent.m_ScriptProperties.size(); i++)
		{
			const ScriptProperty* scriptProperty = &scriptComponent.m_ScriptProperties[i];
			if (std::string_view{ scriptProperty->m_Name }.empty()) continue;
			changedScriptProp |= DrawProperty(*scriptProperty, scriptComponent.m_ScriptData.m_Buffer, 0);
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
