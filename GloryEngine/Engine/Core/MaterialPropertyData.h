#pragma once
#include <any>
#include <string>
#include <typeindex>
#include "Debug.h"

namespace Glory
{
	struct MaterialPropertyData
	{
	public:
		MaterialPropertyData();
		MaterialPropertyData(const MaterialPropertyData& other);
		MaterialPropertyData(const std::string& name);
		MaterialPropertyData(const std::string& name, const std::any& data, uint32_t flags = 0);

		const std::string& Name() const;
		const std::type_index Type() const;
		uint32_t Flags() const;
		std::any& Data();

	private:
		const std::string m_PropertyName;
		uint32_t m_Flags;
		std::any m_PropertyData;
	};
}
