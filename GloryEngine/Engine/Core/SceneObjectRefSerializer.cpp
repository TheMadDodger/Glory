#include "SceneObjectRefSerializer.h"

namespace Glory
{
	void SceneObjectRefSerializer::Serialize(const std::string& name, const std::vector<char>& buffer, uint32_t typeHash, size_t offset, size_t size, Utils::NodeValueRef node)
	{
		SceneObjectRef value;
		memcpy((void*)&value, (void*)&buffer[offset], size);
		node[name].Set(value);
	}

	void SceneObjectRefSerializer::Deserialize(std::vector<char>& buffer, size_t offset, size_t size, Utils::NodeValueRef node)
	{
		if (!node.Exists() || !node.IsMap()) return;
		SceneObjectRef value = node.As<SceneObjectRef>();
		memcpy((void*)&buffer[offset], (void*)&value, size);
	}

	void SceneObjectRefSerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		SceneObjectRef* value = (SceneObjectRef*)data;

		if (name.empty())
		{
			node.Set(*value);
			return;
		}

		node[name].Set(*value);
	}

	void SceneObjectRefSerializer::Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		if (!node.Exists() || !node.IsMap()) return;
		SceneObjectRef* value = (SceneObjectRef*)data;
		*value = node.As<SceneObjectRef>();
	}
}
