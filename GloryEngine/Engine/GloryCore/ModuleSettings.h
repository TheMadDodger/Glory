#pragma once
#include "ResourceType.h"
#include <yaml-cpp/yaml.h>
#include <map>
#include <sstream>
#include <Reflection.h>

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
		void RegisterEnumValue(const std::string& name, T defaultValue)
		{
			YAML::Node node = m_SettingsNode[name];
			if (!node.IsDefined())
			{
				std::string stringValue = "";
				GloryReflect::Enum<T>().ToString(defaultValue, stringValue);
				m_SettingsNode[name] = stringValue;
			}

			RegisterValue(name, ST_Enum, ResourceType::GetHash<T>());
		}

		template<typename T>
		void RegisterArray(const std::string& name)
		{
			YAML::Node node = m_SettingsNode[name];
			if (!node.IsDefined())
				m_SettingsNode[name] = YAML::Node(YAML::NodeType::Sequence);

			if (m_Types.find(name) != m_Types.end()) return;
			RegisterValue(name, ST_Array, ResourceType::GetHash<T>());
		}

		template<typename T>
		T Value(const std::string& name)
		{
			YAML::Node node = m_SettingsNode[name];
			if (!node.IsDefined())
			{
				std::stringstream stream;
				stream << "ModuleSettings: Value " << name << " does not exist";
				throw new std::exception(stream.str().c_str());
			}

			return node.as<T>();
		}

		template<typename T>
		T EnumValue(const std::string& name)
		{
			YAML::Node node = m_SettingsNode[name];
			if (!node.IsDefined())
			{
				std::stringstream stream;
				stream << "ModuleSettings: Value " << name << " does not exist";
				throw new std::exception(stream.str().c_str());
			}

			const std::string strValue = node.as<std::string>();
			T value = T(0);
			GloryReflect::Enum<T>().FromString(strValue, value);
			return value;
		}

		template<typename T>
		T ArrayValue(const std::string& name, const size_t index)
		{
			YAML::Node node = m_SettingsNode[name];
			if (!node.IsSequence())
			{
				std::stringstream stream;
				stream << "ModuleSettings: Value " << name << " is not an array";
				throw new std::exception(stream.str().c_str());
			}

			return node[index].as<T>();
		}

		size_t ArraySize(const std::string& name) const;

		void PushGroup(const std::string& name);
		std::vector<std::string>::iterator GroupsBegin();
		std::vector<std::string>::iterator GroupsEnd();

		std::vector<std::string>::iterator Begin(const std::string& group);
		std::vector<std::string>::iterator End(const std::string& group);

		const uint32_t Type(const std::string& name) const;
		const uint32_t ElementType(const std::string& name) const;
		YAML::Node& Node();

		const bool HasSettings() const;

	private:
		template<typename T>
		void RegisterValue(const std::string& name)
		{
			if (m_Types.find(name) != m_Types.end()) return;
			const uint32_t type = ResourceType::GetHash<T>();
			RegisterValue(name, type, type);
		}

		void RegisterValue(const std::string& name, uint32_t type, uint32_t elementType);

	private:
		YAML::Node m_SettingsNode;
		std::vector<std::string> m_ValueNames;
		std::vector<std::string> m_GroupNames;

		std::map<std::string, uint32_t> m_Types;
		std::map<std::string, uint32_t> m_ElementTypes;
		std::map<std::string, std::vector<std::string>> m_Groups;
	};
}
