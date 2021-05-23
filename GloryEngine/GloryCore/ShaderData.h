#pragma once
#include "Resource.h"
#include <string>
#include <vector>

namespace Glory
{
	class ShaderData : public Resource
	{
	public:
		std::vector<uint32_t>::const_iterator Begin();
		std::vector<uint32_t>::const_iterator End();

		const uint32_t* Data();
		size_t Size();

	private:
		friend class ShaderLoaderModule;
		//std::string m_ProcessedSource;
		//std::vector<std::string> m_IncludedFiles;
		//size_t m_Version;
		std::vector<uint32_t> m_ShaderData;
	};
}