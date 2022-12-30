#include "pch.h"
#include "TemplateManager.h"

namespace Glory::EditorLauncher
{
	std::vector<ProjectTemplate> TemplateManager::m_Templates;

	ProjectTemplate::ProjectTemplate(const std::string& name, const std::string icon, const std::filesystem::path& path)
		: m_Name(name), m_Icon(icon), m_Path(path) {}

	void TemplateManager::LoadTemplates(const EditorInfo& editorInfo)
	{
		m_Templates.clear();

		std::filesystem::path templatesPath = editorInfo.RootPath;
		templatesPath.append("Templates");

		if (!std::filesystem::exists(templatesPath)) return;

		/* Search templates folder for templates */
		for (const auto& directory : std::filesystem::directory_iterator(templatesPath))
		{
			/* Templates should be inside their own subfolders */
			if (!directory.is_directory()) continue;

			std::filesystem::path templatePath = directory.path();
			/* Find .gproj file */
			for (const auto& templateDir : std::filesystem::directory_iterator(templatePath))
			{
				if (templateDir.is_directory()) continue;
				std::filesystem::path filePath = templateDir.path();
				if (filePath.extension() != ".gproj") continue;

				/* TODO: Load icon file */
				m_Templates.push_back({ filePath.filename().replace_extension("").string(), "", templatePath});
				break;
			}
		}
	}

	void TemplateManager::InstantiateTemplate(const std::filesystem::path& pathToCopyFilesTo, const std::string_view projectName, size_t templateIndex)
	{
		const ProjectTemplate& projectTemplate = m_Templates[templateIndex];
		std::filesystem::copy(projectTemplate.m_Path, pathToCopyFilesTo, std::filesystem::copy_options::recursive);
		std::filesystem::path projectFilePath = pathToCopyFilesTo;
		projectFilePath = projectFilePath.append(projectTemplate.m_Name).replace_extension(".gproj");
		std::filesystem::path newProjectFilePath = pathToCopyFilesTo;
		newProjectFilePath = newProjectFilePath.append(projectName).replace_extension(".gproj");
		std::filesystem::rename(projectFilePath, newProjectFilePath);
	}

	void TemplateManager::Clear()
	{
		m_Templates.clear();
	}

	const size_t TemplateManager::TemplateCount()
	{
		return m_Templates.size();
	}

	const ProjectTemplate* TemplateManager::GetTemplate(const size_t index)
	{
		if (index >= m_Templates.size()) return nullptr;
		return &m_Templates[index];
	}
}
