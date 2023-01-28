#pragma once
#include <string>
#include <vector>
#include <map>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace Glory
{
	struct VersionValue
	{
		VersionValue(const std::string& name, const std::string& value);

		std::string Name;
		std::string Value;
	};

	struct Version
	{
	public:
		Version();
		Version(const VersionValue* values, size_t size);
		virtual ~Version();

		const std::string& GetVersionString() const;

		const VersionValue& operator[](size_t index);
		const VersionValue& operator[](const std::string& name);

		//static int Compatible(const Version& a, const Version& b);

		void FromString(const std::string& versionString);

		int HardCompare(const Version& other) const;

		bool IsValid() const;

	private:
		std::vector<VersionValue> m_VersionValues;
		std::map<std::string, size_t> m_Pairs;
		std::string m_VersionString;

		static const char SEPARATOR;
	};
}
