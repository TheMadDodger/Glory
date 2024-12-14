#pragma once
#include "Assembly.h"
#include "ScriptProperty.h"

#include <vector>
#include <string>
#include <mono/metadata/object-forward.h>
#include <Glory.h>
#include <UUID.h>

namespace Glory
{
	struct ScriptProperty;
	class CoreLibManager;

	/** @brief Scripting types manager */
	class MonoScriptManager
	{
	public:
		/** @brief Initialize the manager by retrieving all loaded script component types */
		GLORY_API void Initialize(AssemblyClass* pEngineClass, MonoObject* pEngineObject);
		/** @brief Find the index of a type by name */
		GLORY_API int TypeIndexFromName(std::string_view name) const;
		/** @brief Find the index of a type by hash */
		GLORY_API int TypeIndexFromHash(uint32_t hash) const;
		/** @brief Get a dummy object for a type index */
		GLORY_API MonoObject* Dummy(size_t index) const;
		/** @brief Get the type name of a type index */
		GLORY_API std::string_view TypeName(size_t index) const;
		/** @brief Get the type hash of a type index */
		GLORY_API uint32_t TypeHash(size_t index) const;
		/** @brief Get the number of types */
		GLORY_API size_t TypeCount() const;

		/** @brief Resize a property values array and copy the default values of the script into it */
		GLORY_API void ReadDefaults(size_t typeIndex, std::vector<char>& dest) const;

		/** @brief Get the script properties of a script type */
		GLORY_API const std::vector<ScriptProperty>& ScriptProperties(size_t typeIndex) const;

		/** @brief Write property values to MonoObject */
		GLORY_API void SetPropertyValues(size_t typeIndex, MonoObject* pMonoObject, std::vector<char>& data) const;
		/** @brief Read property values from MonoObject */
		GLORY_API void GetPropertyValues(size_t typeIndex, MonoObject* pMonoObject, std::vector<char>& data) const;
		/** @brief Invoke a method on a script */
		GLORY_API void Invoke(size_t typeIndex, MonoObject* pMonoObject, const std::string& method, void** args);

	public:
		/** @brief Constructor */
		MonoScriptManager(CoreLibManager* pLibManager): m_pCoreLibManager(pLibManager) {}
		/** @brief Destructor */
		virtual ~MonoScriptManager() = default;

	private:
		void LoadScriptProperties(size_t index, AssemblyClass& cls);

	private:
		CoreLibManager* m_pCoreLibManager;
		std::vector<uint32_t> m_ScriptDummyHandles;
		std::vector<std::string> m_ScriptTypeNames;
		std::vector<uint32_t> m_ScriptTypeHashes;
		std::vector<AssemblyClass> m_ScriptClasses;
		std::vector<std::vector<ScriptProperty>> m_ScriptProperties;
		std::vector<std::vector<char>> m_DefaultValues;
	};
}
