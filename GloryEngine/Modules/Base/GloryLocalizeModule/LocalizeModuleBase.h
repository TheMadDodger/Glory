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

		/** @brief Find a string in the loaded tables
		 * @param tableName Name of the table to search in
		 * @param term Key of the string to find in the table
		 * @param out Output to write the result to if the term is found
		 * @returns true if the term was found, false otherwise
		 */
		virtual bool FindString(const std::string_view tableName, const std::string_view term, std::string& out) = 0;

		std::function<void()> OnLanguageChanged;
    };
}
