#pragma once
#include <vector>
#include "EditorManager.h"

namespace Glory::EditorLauncher
{
	struct ProjectTemplate
	{
	public:
		ProjectTemplate(const std::string& name, const std::string icon, const std::filesystem::path& path);

		const std::string m_Name;
		const std::filesystem::path m_Path;
		const std::string m_Icon;
	};

	class TemplateManager
	{
	public:
		static void LoadTemplates(const EditorInfo& editorInfo);
		static void InstantiateTemplate(const std::filesystem::path& pathToCopyFilesTo, const std::string_view projectName, size_t templateIndex);
		static void Clear();

		static const size_t TemplateCount();
		static const ProjectTemplate* GetTemplate(const size_t index);

	private:
		static std::vector<ProjectTemplate> m_Templates;
	};
}
