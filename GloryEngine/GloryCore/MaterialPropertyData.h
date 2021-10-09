#pragma once
#include <string>

namespace Glory
{
	class MaterialPropertyData
	{
	public:
		MaterialPropertyData(const std::string& propertyName);
		virtual ~MaterialPropertyData();

	private:
		const std::string m_PropertyName;
	};
}
