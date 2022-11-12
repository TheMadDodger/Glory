#include "ScriptedComponentEditor.h"
#include "Undo.h"
#include "GizmoAction.h"
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
		m_pScript = AssetManager::GetAssetImmediate<Script>(scriptComponent.m_Script.AssetUUID());
		if (!m_pScript)
		{
			scriptComponent.m_Script.SetUUID(0);
			return;
		}

		//scriptComponent.m_ScriptData = YAML::Node(YAML::NodeType::Sequence);
		//YAML::Node dataNode{ YAML::NodeType::Map };
		//dataNode.force_insert("Name", "_angularSpeed");
		//dataNode.force_insert("Value", 1.0f);
		//scriptComponent.m_ScriptData.push_back(dataNode);

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

		bool changedScriptProp = false;
		for (size_t i = 0; i < scriptComponent.m_ScriptProperties.size(); i++)
		{
			const ScriptProperty* scriptProperty = &scriptComponent.m_ScriptProperties[i];
			changedScriptProp |= PropertyDrawer::DrawProperty(*scriptProperty, scriptComponent.m_ScriptData, 0);
		}

		if (changedScriptProp) Validate();
		return change || changedScriptProp;

		//if (!scriptComponent.m_ScriptData.IsDefined()) return change;




		//YAML::Node scriptData{ YAML::NodeType::Sequence };
		//
		//for (size_t i = 0; i < scriptComponent.m_ScriptData.size(); i++)
		//{
		//	YAML::Node fieldNode = scriptComponent.m_ScriptData[i];
		//	std::string fieldName = fieldNode["Name"].as<std::string>();
		//	float value = fieldNode["Value"].as<float>();
		//
		//	YAML::Node newNode{ YAML::NodeType::Map };
		//	newNode.force_insert("Name", fieldName);
		//	change |= PropertyDrawer::DrawProperty(fieldName, &value, ResourceType::GetHash<float>(), 0);
		//	newNode.force_insert("Value", value);
		//	scriptData.push_back(newNode);
		//}
		//
		//scriptComponent.m_ScriptData = scriptData;


		//UpdateTransform();
		//if (change) m_pGizmo->UpdateTransform(m_Transform);
		//glm::mat4 oldTransform;
		//if (!m_pGizmo->WasManipulated(oldTransform, m_Transform)) return change;
		//
		//Undo::StartRecord("Transform");
		//Undo::AddAction(new GizmoAction(m_pTarget->GetUUID(), oldTransform, m_Transform));
		//Undo::StopRecord();
		//return true;
	}

	std::string ScriptedComponentEditor::Name()
	{
		return "Script";
	}
}