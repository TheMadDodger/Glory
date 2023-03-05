#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <yaml-cpp/yaml.h>

namespace Glory
{
	enum class ModuleType
	{
		MT_Window,
		MT_Graphics,
		MT_Renderer,
		MT_SceneManagement,
		MT_Loader,
		MT_Scripting,
		MT_Input,
		MT_Physics,
		MT_Other,
	};

	struct ModuleScriptingExtension
	{
		std::string m_Language;
		std::string m_ExtensionFile;
	};

	class ModuleMetaData
	{
	public:
		ModuleMetaData();
		ModuleMetaData(const std::filesystem::path& path);
		virtual ~ModuleMetaData();

		void Read();

		const std::filesystem::path& Path() const;
		const std::string& Name() const;
		const ModuleType& Type() const;
		const std::string& EditorBackend() const;
		const std::vector<std::string>& EditorExtensions() const;
		const std::vector<std::string>& Dependencies() const;
		const ModuleScriptingExtension* const ScriptExtenderForLanguage(const std::string& language) const;

	private:
		void ReadScriptingExtender(YAML::Node& node);

	private:
		std::filesystem::path m_Path;
		std::string m_Name;
		ModuleType m_Type;
		std::string m_EditorBackend;
		std::vector<std::string> m_EditorExtensions;
		std::vector<std::string> m_Dependencies;
		std::map<std::string, ModuleScriptingExtension> m_ScriptingExtensions;

		static std::map<std::string, ModuleType> STRINGTOMODULETYPE;
	};
}