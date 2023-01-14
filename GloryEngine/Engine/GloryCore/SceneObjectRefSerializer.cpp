#include "SceneObjectRefSerializer.h"

namespace Glory
{
	void SceneObjectRefSerializer::Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out)
	{
		SceneObjectRef value = *(SceneObjectRef*)serializedProperty->MemberPointer();
		out << YAML::Key << serializedProperty->Name();
		out << YAML::Value << value;
	}

	void SceneObjectRefSerializer::Serialize(const std::string& name, const std::vector<char>& buffer, size_t typeHash, size_t offset, size_t size, YAML::Emitter& out)
	{
		SceneObjectRef value;
		memcpy((void*)&value, (void*)&buffer[offset], size);
		out << YAML::Key << name;
		out << YAML::Value << value;
	}

	void SceneObjectRefSerializer::Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		const std::string& name = pFieldData->Name();

		SceneObjectRef value;
		pFieldData->Get(data, &value);

		if (name == "")
		{
			out << value;
			return;
		}

		out << YAML::Key << name;
		out << YAML::Value << value;
	}

	void SceneObjectRefSerializer::Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		SceneObjectRef* pMember = (SceneObjectRef*)serializedProperty->MemberPointer();
		SceneObjectRef data = object.as<SceneObjectRef>();
		*pMember = data;
	}

	void SceneObjectRefSerializer::Deserialize(std::any& out, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		out = object.as<SceneObjectRef>();
	}

	void SceneObjectRefSerializer::Deserialize(std::vector<char>& buffer, size_t offset, size_t size, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		SceneObjectRef value = object.as<SceneObjectRef>();
		memcpy((void*)&buffer[offset], (void*)&value, size);
	}

	void SceneObjectRefSerializer::Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		SceneObjectRef value = object.as<SceneObjectRef>();
		pFieldData->Set(data, &value);
	}
}
