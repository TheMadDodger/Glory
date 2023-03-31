#include "SceneObjectRefSerializer.h"

namespace Glory
{
	void SceneObjectRefSerializer::Serialize(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, YAML::Emitter& out)
	{
		SceneObjectRef value;
		memcpy((void*)&value, (void*)&buffer[offset], size);
		out << YAML::Key << name;
		out << YAML::Value << value;
	}

	void SceneObjectRefSerializer::Deserialize(std::vector<char>& buffer, size_t offset, size_t size, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		SceneObjectRef value = object.as<SceneObjectRef>();
		memcpy((void*)&buffer[offset], (void*)&value, size);
	}

	void SceneObjectRefSerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		SceneObjectRef* value = (SceneObjectRef*)data;

		if (name.empty())
		{
			out << *value;
			return;
		}

		out << YAML::Key << name;
		out << YAML::Value << *value;
	}

	void SceneObjectRefSerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		SceneObjectRef* value = (SceneObjectRef*)data;
		*value = object.as<SceneObjectRef>();
	}
}
