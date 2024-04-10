#include "ScriptedComponentSerializer.h"
#include "PropertySerializer.h"
#include "MonoComponents.h"

#include <Engine.h>

namespace Glory
{
	ScriptedComponentSerailizer::ScriptedComponentSerailizer(Serializers* pSerializers):
		PropertySerializer(pSerializers, ResourceTypes::GetHash<MonoScriptComponent>())
	{
	}

	ScriptedComponentSerailizer::~ScriptedComponentSerailizer()
	{
	}

	void ScriptedComponentSerailizer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		MonoScriptComponent* pScriptedComponent = (MonoScriptComponent*)data;
		out << YAML::Key << "Properties";
		out << YAML::BeginMap;
		out << YAML::Key << "m_Script" << YAML::Value << pScriptedComponent->m_Script.AssetUUID();
		out << YAML::Key << "ScriptData" << YAML::Value << YAML::BeginMap;

		MonoScript* pScript = pScriptedComponent->m_Script.GetImmediate(&m_pSerializers->GetEngine()->GetAssetManager());
		if (pScript)
		{
			pScript->LoadScriptProperties();

			const std::vector<ScriptProperty>& props = pScript->ScriptProperties();
			for (size_t i = 0; i < props.size(); ++i)
			{
				const ScriptProperty& prop = props[i];
				const TypeData* pType = Reflect::GetTyeData(prop.m_TypeHash);
				m_pSerializers->SerializeProperty(prop.m_Name, pType, &pScriptedComponent->m_ScriptData.m_Buffer[prop.m_RelativeOffset], out);
			}
		}

		out << YAML::EndMap << YAML::EndMap;
	}

	void ScriptedComponentSerailizer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		YAML::Node scriptNode = object["m_Script"];
		YAML::Node scriptDataNode = object["ScriptData"];

		MonoScriptComponent* pScriptedComponent = (MonoScriptComponent*)data;
		pScriptedComponent->m_Script.SetUUID(scriptNode.as<uint64_t>());
		MonoScript* pScript = pScriptedComponent->m_Script.GetImmediate(&m_pSerializers->GetEngine()->GetAssetManager());
		if (pScript)
		{
			pScript->LoadScriptProperties();
			pScript->ReadDefaults(pScriptedComponent->m_ScriptData.m_Buffer);

			const std::vector<ScriptProperty>& props = pScript->ScriptProperties();
			for (size_t i = 0; i < props.size(); ++i)
			{
				const ScriptProperty& prop = props[i];
				const TypeData* pType = Reflect::GetTyeData(prop.m_TypeHash);
				m_pSerializers->DeserializeProperty(pType, &pScriptedComponent->m_ScriptData.m_Buffer[prop.m_RelativeOffset], scriptDataNode[prop.m_Name]);
			}
		}
	}
}
