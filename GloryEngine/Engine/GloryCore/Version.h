#pragma once
#include <string>

namespace Glory
{
	struct Version
	{
	public:
		Version() : v{0, 0, 0, 0} {}
		constexpr Version(int major, int minor, int subMinor, int rc) : v{major, minor, subMinor, rc} {}

		static Version Parse(const char* str);
		static int Compare(const Version& v1, const Version& v2);

		static constexpr char Separator = '.';

		int operator[](size_t index) const;
		bool IsValid() const;
		void GetVersionString(std::string& out) const;

		union
		{
			struct
			{
				int Major;
				int Minor;
				int SubMinor;
				int RC;
			};
			int v[4];
		};
	};
}