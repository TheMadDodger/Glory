#pragma once
#include "UUID.h"
#include "Glory.h"

#include <typeindex>
#include <string>

#define YAML_READ(startNode, node, key, out, type) node = startNode[#key]; \
if (node.IsDefined()) out = node.as<type>()

#define YAML_WRITE(emitter, key, value) emitter << YAML::Key << #key << YAML::Value << value;

#define APPEND_TYPE(x) PushInheritence<x>()

namespace Glory
{
	class Object
	{
	public:
		Object();
		Object(UUID uuid);
		Object(const std::string& name);
		Object(UUID uuid, const std::string& name);
		virtual ~Object();

		UUID GetUUID() const;
		virtual UUID GetGPUUUID() const;

		size_t TypeCount() const;
		bool GetType(size_t index, std::type_index& type) const;

		const std::string& Name() const;
		void SetName(const std::string& name);
		void SetName(const std::string_view name);

		virtual void* GetRootDataAddress();

	protected:
		std::vector<std::type_index> m_Inheritence;

		template<class T>
		void PushInheritence()
		{
			PushInheritence(typeid(T));
		}

		void PushInheritence(const std::type_index& type);

	protected:
		UUID m_ID;
		std::string m_Name;

	private:
		friend class AssetDatabase;
	};
}