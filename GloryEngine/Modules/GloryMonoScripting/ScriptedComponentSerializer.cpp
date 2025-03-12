#include "ScriptedComponentSerializer.h"
#include "PropertySerializer.h"
#include "MonoComponents.h"
#include "MonoScriptManager.h"
#include "MonoManager.h"
#include "CoreLibManager.h"
#include "GloryMonoScipting.h"
#include "AssetManager.h"

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
		node["m_ScriptType"].Set(pScriptedComponent->m_ScriptType.m_Hash);
		auto scriptData = node["ScriptData"];
		scriptData.Set(YAML::Node(YAML::NodeType::Map));

		MonoScriptManager& scriptManager = MonoManager::Instance()->GetCoreLibManager()->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pScriptedComponent->m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		const std::vector<ScriptProperty>& props = scriptManager.ScriptProperties((size_t)typeIndex);
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

	void ScriptedComponentSerailizer::Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		auto scriptNode = node["m_Script"];
		auto scriptTypeNode = node["m_ScriptType"];
		auto scriptDataNode = node["ScriptData"];

		/* Migrate old script type */
		if (scriptNode.Exists())
		{
			const UUID scriptID = scriptNode.As<uint64_t>();
			Resource* pScriptResource = MonoManager::Instance()->Module()->GetEngine()->GetAssetManager().GetAssetImmediate(scriptID);
			if (pScriptResource)
			{
				MonoScript* pScript = (MonoScript*)pScriptResource;
				const std::string_view fullName = pScript->ClassName(0);
				const uint32_t hash = Hashing::Hash(fullName.data());
				scriptTypeNode.Set(hash);
			}
		}

		MonoScriptComponent* pScriptedComponent = (MonoScriptComponent*)data;
		pScriptedComponent->m_ScriptType.m_Hash = scriptTypeNode.As<uint32_t>(0);

		MonoScriptManager& scriptManager = MonoManager::Instance()->GetCoreLibManager()->ScriptManager();
		int typeIndex = scriptManager.TypeIndexFromHash(pScriptedComponent->m_ScriptType.m_Hash);
		if (typeIndex == -1) return;

		scriptManager.ReadDefaults((size_t)typeIndex, pScriptedComponent->m_ScriptData.m_Buffer);
		if (!scriptDataNode.Exists() && !scriptDataNode.IsMap()) return;

		const std::vector<ScriptProperty>& props = scriptManager.ScriptProperties((size_t)typeIndex);
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
