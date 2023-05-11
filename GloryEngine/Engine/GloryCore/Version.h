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
		/** @brief Compare 2 versions
		 * @param v1 Version 1
		 * @param v2 Version 2
		 * @returns -1 if v1 is less than v2, +1 for the opposite, 0 if they are identical
		 */
		static int Compare(const Version& v1, const Version& v2, bool ignoreRC = false);

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