#pragma once
#include "ResourceType.h"
#include <yaml-cpp/yaml.h>
#include <map>

namespace Glory
{
	class ModuleSettings
	{
	public:
		ModuleSettings();
		ModuleSettings(YAML::Node& settingsNode);
		virtual ~ModuleSettings();

		template<typename T>
		void RegisterValue(const std::string& name, T defaultValue)
		{
			YAML::Node node = m_SettingsNode[name];
			if (!node.IsDefined())
				m_SettingsNode[name] = defaultValue;

			RegisterValue<T>(name);
		}

		template<typename T>
		T Value(const std::string& name)
		{
			YAML::Node node = m_SettingsNode[name];
			if (!node.IsDefined())
				throw new std::exception("ModuleSettings: Value " + name + " does not exist");

			return node.as<T>();
		}

		void PushGroup(const std::string& name);
		std::vector<std::string>::iterator GroupsBegin();
		std::vector<std::string>::iterator GroupsEnd();

		std::vector<std::string>::iterator Begin(const std::string& group);
		std::vector<std::string>::iterator End(const std::string& group);

		const uint32_t Type(const std::string& name) const;
		YAML::Node& Node();

	private:
		template<typename T>
		void RegisterValue(const std::string& name)
		{
			if (m_Types.find(name) != m_Types.end()) return;
			RegisterValue(name, ResourceType::GetHash<T>());
		}

		void RegisterValue(const std::string& name, uint32_t type);

	private:
		YAML::Node m_SettingsNode;
		std::vector<std::string> m_ValueNames;
		std::vector<std::string> m_GroupNames;

		std::map<std::string, uint32_t> m_Types;
		std::map<std::string, std::vector<std::string>> m_Groups;
	};
}
