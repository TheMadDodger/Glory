#include "ModuleSettings.h"

namespace Glory
{
	ModuleSettings::ModuleSettings()
		: m_SettingsNode(0), m_Dirty(false) {}

	ModuleSettings::ModuleSettings(YAML::Node& settingsNode)
		: m_SettingsNode(settingsNode), m_GroupNames({ "General" }),
		m_Groups({ {"General", std::vector<std::string>()}}), m_Dirty(false) {}

	ModuleSettings::~ModuleSettings() {}

	size_t ModuleSettings::ArraySize(const std::string& name) const
	{
		YAML::Node node = m_SettingsNode[name];
		if (!node.IsSequence())
		{
			std::stringstream stream;
			stream << "ModuleSettings: Value " << name << " is not an array";
			throw new std::exception(stream.str().c_str());
		}

		return node.size();
	}

	void ModuleSettings::PushGroup(const std::string& name)
	{
		m_GroupNames.push_back(name);
	}

	std::vector<std::string>::iterator ModuleSettings::GroupsBegin()
	{
		return m_GroupNames.begin();
	}

	std::vector<std::string>::iterator ModuleSettings::GroupsEnd()
	{
		return m_GroupNames.end();
	}

	std::vector<std::string>::iterator ModuleSettings::Begin(const std::string& group)
	{
		return m_Groups.at(group).begin();
	}

	std::vector<std::string>::iterator ModuleSettings::End(const std::string& group)
	{
		return m_Groups.at(group).end();
	}

	const uint32_t ModuleSettings::Type(const std::string& name) const
	{
		if (m_Types.find(name) == m_Types.end()) return 0;
		return m_Types.at(name);
	}

	const uint32_t ModuleSettings::ElementType(const std::string& name) const
	{
		if (m_ElementTypes.find(name) == m_ElementTypes.end()) return 0;
		return m_ElementTypes.at(name);
	}

	YAML::Node& ModuleSettings::Node()
	{
		return m_SettingsNode;
	}

	const bool ModuleSettings::HasSettings() const
	{
		return m_ValueNames.size();
	}

	void ModuleSettings::SetDirty(bool dirty)
	{
		m_Dirty = dirty;
	}

	bool ModuleSettings::IsDirty() const
	{
		return m_Dirty;
	}

	void ModuleSettings::RegisterValue(const std::string& name, uint32_t type, uint32_t elementType)
	{
		m_ValueNames.push_back(name);
		m_Types.emplace(m_ValueNames.back(), type);
		m_ElementTypes.emplace(m_ValueNames.back(), elementType);
		const std::string& group = m_GroupNames.back();
		if (m_Groups.find(group) == m_Groups.end())
			m_Groups.emplace(group, std::vector<std::string>());
		m_Groups[group].push_back(m_ValueNames.back());
	}
}
