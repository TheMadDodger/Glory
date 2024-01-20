#include "ScriptedComponentSerializer.h"
#include "PropertySerializer.h"
#include "MonoComponents.h"

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
		out << YAML::Key << "ScriptData" << YAML::Value << pScriptedComponent->m_ScriptData;
		out << YAML::EndMap;
	}

	void ScriptedComponentSerailizer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		YAML::Node scriptNode = object["m_Script"];

		MonoScriptComponent* pScriptedComponent = (MonoScriptComponent*)data;
		pScriptedComponent->m_Script.SetUUID(scriptNode.as<uint64_t>());
		pScriptedComponent->m_ScriptData = object["ScriptData"];
	}
}
