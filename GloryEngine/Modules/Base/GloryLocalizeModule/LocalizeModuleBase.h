#pragma once
#include <Module.h>

#include <functional>

namespace Glory
{
	/** @brief Base class for localization modules */
    class LocalizeModuleBase : public Module
    {
	public:
		/** @brief Constructor */
		LocalizeModuleBase();
		/** @brief Destructor */
		virtual ~LocalizeModuleBase();

		/** @brief LocalizeModuleBase type */
		const std::type_info& GetBaseModuleType() override;

		virtual bool FindString(const std::string_view tableName, const std::string_view term, std::string& out) = 0;

    };
}
