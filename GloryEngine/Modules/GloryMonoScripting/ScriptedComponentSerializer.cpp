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

	void ScriptedComponentSerailizer::Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		MonoScriptComponent* pScriptedComponent = (MonoScriptComponent*)data;
		node.Set(YAML::Node(YAML::NodeType::Map));
		node["m_Script"].Set(uint64_t(pScriptedComponent->m_Script.AssetUUID()));
		auto scriptData = node["ScriptData"];
		scriptData.Set(YAML::Node(YAML::NodeType::Map));

		MonoScript* pScript = pScriptedComponent->m_Script.GetImmediate(&m_pSerializers->GetEngine()->GetAssetManager());
		if (pScript)
		{
			pScript->LoadScriptProperties();

			const std::vector<ScriptProperty>& props = pScript->ScriptProperties();
			for (size_t i = 0; i < props.size(); ++i)
			{
				const ScriptProperty& prop = props[i];
				switch (prop.m_TypeHash)
				{
				case ST_Object:
				{
					PropertySerializer* pSerializer = m_pSerializers->GetSerializer(ST_Object);
					pSerializer->Serialize(&pScriptedComponent->m_ScriptData.m_Buffer[prop.m_RelativeOffset], prop.m_ElementTypeHash, scriptData[prop.m_Name]);
					break;
				}
				case ST_Asset:
				{
					PropertySerializer* pSerializer = m_pSerializers->GetSerializer(ST_Asset);
					pSerializer->Serialize(&pScriptedComponent->m_ScriptData.m_Buffer[prop.m_RelativeOffset], prop.m_ElementTypeHash, scriptData[prop.m_Name]);
					break;
				}
				default:
				{
					const TypeData* pType = Reflect::GetTyeData(prop.m_TypeHash);
					m_pSerializers->SerializeProperty(pType, &pScriptedComponent->m_ScriptData.m_Buffer[prop.m_RelativeOffset], scriptData[prop.m_Name]);
					break;
				}
				}
			}
		}
	}

	void ScriptedComponentSerailizer::Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		auto scriptNode = node["m_Script"];
		auto scriptDataNode = node["ScriptData"];

		MonoScriptComponent* pScriptedComponent = (MonoScriptComponent*)data;
		pScriptedComponent->m_Script.SetUUID(scriptNode.As<uint64_t>());
		MonoScript* pScript = pScriptedComponent->m_Script.GetImmediate(&m_pSerializers->GetEngine()->GetAssetManager());
		if (pScript)
		{
			pScript->LoadScriptProperties();
			pScript->ReadDefaults(pScriptedComponent->m_ScriptData.m_Buffer);

			if (!scriptDataNode.Exists() && !scriptDataNode.IsMap()) return;

			const std::vector<ScriptProperty>& props = pScript->ScriptProperties();
			for (size_t i = 0; i < props.size(); ++i)
			{
				const ScriptProperty& prop = props[i];
				auto propData = scriptDataNode[prop.m_Name];
				if (!propData.Exists()) return;
				switch (prop.m_TypeHash)
				{
				case ST_Object:
				{
					PropertySerializer* pSerializer = m_pSerializers->GetSerializer(ST_Object);
					pSerializer->Deserialize(&pScriptedComponent->m_ScriptData.m_Buffer[prop.m_RelativeOffset], prop.m_ElementTypeHash, propData);
					break;
				}
				case ST_Asset:
				{
					PropertySerializer* pSerializer = m_pSerializers->GetSerializer(ST_Asset);
					pSerializer->Deserialize(&pScriptedComponent->m_ScriptData.m_Buffer[prop.m_RelativeOffset], prop.m_ElementTypeHash, propData);
					break;
				}
				default:
				{
					const TypeData* pType = Reflect::GetTyeData(prop.m_TypeHash);
					m_pSerializers->DeserializeProperty(pType, &pScriptedComponent->m_ScriptData.m_Buffer[prop.m_RelativeOffset], propData);
					break;
				}
				}
			}
		}
	}
}
