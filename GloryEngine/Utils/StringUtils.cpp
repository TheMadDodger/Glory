#include "StringUtils.h"

#include <algorithm>
#include <string>

namespace Glory::Utils
{
	struct CaseInsensitiveStringEqual
	{
		bool operator()(char ch1, char ch2)
		{
			return std::toupper(ch1) == std::toupper(ch2);
		}
	};

	size_t CaseInsensitiveSearch(const std::string_view str1, const std::string_view str2)
	{
		const auto it = std::search(str1.begin(), str1.end(), str2.begin(), str2.end(), CaseInsensitiveStringEqual());
		if (it == str1.end()) return std::string::npos;
		return it - str1.begin();
	}
}