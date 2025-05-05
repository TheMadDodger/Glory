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
		/** @brief Update the value of a string
		 * @param key The key of the string
		 * @param value The new value of the string
		 */
		GLORY_API void UpdateString(const std::string& key, std::string&& value);
		/** @brief Check if this table contains a sepcific key
		 * @param key The key to check for
		 */
		GLORY_API bool HasKey(const std::string& key) const;
		/** @brief Remove a string from this table
		 * @param key The key of the string to remove
		 */
		GLORY_API void RemoveKey(const std::string& key);
		/** @brief Find all keys belonging to a specific group at a specified path
		 * @param path Path to the group
		 */
		GLORY_API std::vector<std::string>* FindKeys(const std::string& path);
		/** @brief Search for all keys recursively starting at a path
		 * @param path Path to the group to search recursively
		 * @param keys Vector containing all found keys
		 */
		GLORY_API void FindKeysRecursively(const std::string& path, std::vector<std::string>& keys);

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

	protected:
		/** @brief Get a vector containing other resources referenced by this resource */
		virtual void References(Engine*, std::vector<UUID>&) const override;

		virtual void Serialize(BinaryStream& container) const override;
		virtual void Deserialize(BinaryStream& container) override;

	private:
		struct GroupData
		{
			std::map<std::string, GroupData> m_Subgroups;
			std::vector<std::string> m_Keys;
		};
		/** @brief Search for all keys recursively inside a group
		 * @param basePath Base path to add to the start of the key
		 * @param group Group to search keys in
		 * @param keys Vector containing all found keys
		 */
		void FindKeysRecursively(std::string basePath, GroupData* group, std::vector<std::string>& keys);

	private:
		GroupData m_RootGroup;
		std::map<std::string, std::string> m_Strings;
	};
}
