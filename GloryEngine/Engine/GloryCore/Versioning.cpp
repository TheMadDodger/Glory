#include "Versioning.h"

namespace Glory
{
	const char Version::SEPARATOR = '.';

	VersionValue::VersionValue(const std::string& name, const std::string& value) : Name(name), Value(value)
	{

	}

	Version::Version() : m_VersionValues(), m_Pairs(), m_VersionString("?")
	{
	}

	Version::Version(const VersionValue* values, size_t size)
	{
		for (size_t i = 0; i < size; i++)
		{
			m_VersionValues.push_back(values[i]);
			m_Pairs.emplace(values[i].Name, i);
			m_VersionString += values[i].Value + '.';
		}
		m_VersionString.erase(m_VersionString.end() - 1);
	}

	Version::~Version()
	{
		m_VersionValues.clear();
		m_Pairs.clear();
		m_VersionString.clear();
	}

	const std::string& Version::GetVersionString() const
	{
		return m_VersionString;
	}

	const VersionValue& Version::operator[](size_t index)
	{
		return m_VersionValues[index];
	}

	const VersionValue& Version::operator[](const std::string& name)
	{
		size_t index = m_Pairs[name];
		return operator[](index);
	}

	void Version::FromString(const std::string& versionString)
	{
		size_t currentIndex = 0;
		m_VersionString.clear();
		for (size_t i = 0; i < m_VersionValues.size(); i++)
		{
			size_t dotIndex = versionString.find(SEPARATOR, currentIndex);
			if (dotIndex == std::string::npos)
			{
				m_VersionValues[i].Value = versionString.substr(currentIndex);
				m_VersionString += m_VersionValues[i].Value;
				break;
			}
			m_VersionValues[i].Value = versionString.substr(currentIndex, dotIndex - currentIndex);
			currentIndex = dotIndex + 1;
			m_VersionString += m_VersionValues[i].Value + '.';
		}
	}

	int Version::HardCompare(const Version& other) const
	{
		if (other.m_VersionValues.size() == 0) return -1;
		int compareValue = 0;
		for (size_t i = 0; i < m_VersionValues.size(); i++)
		{
			if (other.m_VersionValues.size() <= i) break;
			int factor = (int)pow(10.0, (double)(m_VersionValues.size() - i - 1));
			if (m_VersionValues[i].Value == other.m_VersionValues[i].Value) continue;
			compareValue += factor;
		}
		return compareValue;
	}

	bool Version::IsValid() const
	{
		return !m_VersionValues.empty();
	}
}
