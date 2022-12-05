#include "ScriptedComponentSerializer.h"
#include <PropertySerializer.h>

namespace Glory
{
	void ScriptedComponentSerailizer::Serialize(UUID uuid, ScriptedComponent& component, YAML::Emitter& out)
	{
		const GloryReflect::TypeData* pTypeData = ScriptedComponent::GetTypeData();
		for (size_t i = 0; i < pTypeData->FieldCount(); i++)
		{
			const GloryReflect::FieldData* pFieldData = pTypeData->GetFieldData(i);
			PropertySerializer::SerializeProperty(pFieldData, &component, out);
		}

		out << YAML::Key << "ScriptData" << YAML::Value << component.m_ScriptData;
	}

	void ScriptedComponentSerailizer::Deserialize(ScriptedComponent& component, YAML::Node& object)
	{
		const GloryReflect::TypeData* pTypeData = ScriptedComponent::GetTypeData();
		for (size_t i = 0; i < pTypeData->FieldCount(); i++)
		{
			const GloryReflect::FieldData* pFieldData = pTypeData->GetFieldData(i);
			PropertySerializer::DeserializeProperty(pFieldData, &component, object[pFieldData->Name()]);
		}

		component.m_ScriptData = object["ScriptData"];
	}
}
