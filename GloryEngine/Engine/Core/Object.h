#pragma once
#include "UUID.h"
#include "Glory.h"

#include <typeindex>
#include <string>

/* *IDEA FOR DETECTING OBJECT INHERITENCE AND OBJECT TYPE*
* The Object class keeps track of an array of all the inherited classes,
* the inherited class could add itself to this aray in its constructor,
* could be made easier with the use of a macro that generates the constructor for you,
* this would mean that at index 0 of the array you have the last class of inheritence,
* while the last index is the base class at the top of the hierarchy.
* Now when searching for a specific type you check for index 0,
* and when searching if an object is inheriting a specific class
* you check if that type is present in the array.
*/

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