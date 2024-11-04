#include "MonoScriptComponentEditor.h"
#include "MonoEditorExtension.h"

#include <AssetManager.h>
#include <MonoManager.h>
#include <CoreLibManager.h>
#include <GloryMonoScipting.h>

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
		GloryMonoScipting* pScripting = EditorApplication::GetInstance()->GetEngine()->GetOptionalModule<GloryMonoScipting>();
		const MonoScriptManager& scriptManager = pScripting->GetMonoManager()->GetCoreLibManager()->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);
		if (typeIndex == -1) return;
		scriptManager.ReadDefaults((size_t)typeIndex, scriptComponent.m_ScriptData.m_Buffer);
	}

	bool MonoScriptComponentEditor::OnGUI()
	{
		const size_t compilationVersion = MonoEditorExtension::CompilationVersion();

		bool change = EntityComponentEditor::OnGUI();
		MonoScriptComponent& scriptComponent = GetTargetComponent();
		if (change && scriptComponent.m_ScriptType.m_Hash || m_LastCompilationVersion != compilationVersion)
		{
			Initialize();
			return change;
		}
		GloryMonoScipting* pScripting = EditorApplication::GetInstance()->GetEngine()->GetOptionalModule<GloryMonoScipting>();
		const MonoScriptManager& scriptManager = pScripting->GetMonoManager()->GetCoreLibManager()->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(scriptComponent.m_ScriptType.m_Hash);

		if (typeIndex == -1) return change;

		Undo::StartRecord("Property Change", m_pComponentObject->GetUUID(), true);
		bool changedScriptProp = false;
		const auto& properties = scriptManager.ScriptProperties((size_t)typeIndex);
		for (size_t i = 0; i < properties.size(); i++)
		{
			const ScriptProperty& scriptProperty = properties[i];
			if (std::string_view{ scriptProperty.m_Name }.empty()) continue;
			changedScriptProp |= DrawProperty(scriptProperty, scriptComponent.m_ScriptData.m_Buffer, 0);
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
