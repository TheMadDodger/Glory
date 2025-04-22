#pragma once
#include <Resource.h>

namespace Glory
{
	/** @brief String table resource */
	class StringTable : public Resource
	{
	public:
		/** @brief Constructor */
		GLORY_API StringTable();
		/** @brief Destructor */
		GLORY_API virtual ~StringTable();

		/** @brief Add a string to this table
		 * @param key The lookup key of the string
		 * @param value The value of the string
		 */
		GLORY_API void AddString(std::string&& key, std::string&& value);

		/** @brief String lookup result */
		struct LookupResult
		{
		public:
			GLORY_API operator bool() const { return Valid; }
			GLORY_API const std::string_view operator*() const { return Value; }

		private:
			friend class StringTable;
			LookupResult(bool valid, std::string_view value)
				: Valid(valid), Value(value) {}

			const bool Valid;
			const std::string_view Value;
		};

		/** @brief Find a string in this table
		 * @param key The lookup key of the string
		 * @returns A valid @ref LookupResult if the key was found, invalid otherwise
		 */
		GLORY_API LookupResult FindString(const std::string& key) const;

		/** @brief Begin iterator for terms */
		GLORY_API std::map<std::string, std::string>::const_iterator Begin() const;
		/** @brief End iterator for terms */
		GLORY_API std::map<std::string, std::string>::const_iterator End() const;

	private:
		/** @brief Get a vector containing other resources referenced by this resource */
		virtual void References(Engine*, std::vector<UUID>&) const override;

		virtual void Serialize(BinaryStream& container) const override;
		virtual void Deserialize(BinaryStream& container) override;

	private:
		std::map<std::string, std::string> m_Strings;
	};
}
