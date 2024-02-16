#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace Glory
{
	enum class ModuleType
	{
		MT_Invalid,

		MT_Window,
		MT_Graphics,
		MT_Renderer,
		MT_Input,

		MT_Loader,
		MT_Other,
	};

	struct ModuleExtra
	{
		std::string m_File;
		std::string m_Requires;
	};

	class Debug;

	class ModuleMetaData
	{
	public:
		ModuleMetaData();
		ModuleMetaData(const std::filesystem::path& path);
		virtual ~ModuleMetaData();

		void Read(Debug& debug);

		const std::filesystem::path& Path() const;
		const std::string& Name() const;
		const ModuleType& Type() const;
		const std::string& EditorBackend() const;
		const std::vector<std::string>& EditorExtensions() const;
		const std::vector<std::string>& Dependencies() const;
		const size_t NumExtras() const;
		const ModuleExtra& Extra(size_t index) const;

	private:
		std::filesystem::path m_Path;
		std::string m_Name;
		ModuleType m_Type;
		std::string m_EditorBackend;
		std::vector<std::string> m_EditorExtensions;
		std::vector<std::string> m_Dependencies;
		std::vector<ModuleExtra> m_ModuleExtras;
	};
}