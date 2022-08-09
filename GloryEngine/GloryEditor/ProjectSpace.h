#pragma once
#include <string>
#include <mutex>
#include "GloryEditor.h"

namespace Glory::Editor
{
	class ProjectSpace
	{
	public:
		static GLORY_EDITOR_API void OpenProject(const std::string& path);
		static GLORY_EDITOR_API void CloseProject();
		static GLORY_EDITOR_API ProjectSpace* GetOpenProject();
		static GLORY_EDITOR_API bool ProjectExists(const std::string& path);
		static GLORY_EDITOR_API bool ProjectExists(const std::string& path, const std::string& name);
		static GLORY_EDITOR_API std::string NewProject(const std::string& path, const std::string& name);
		GLORY_EDITOR_API void CreateFolder(const std::string& name);

		GLORY_EDITOR_API std::string Name();
		GLORY_EDITOR_API std::string RootPath();
		GLORY_EDITOR_API std::string ProjectPath();
		GLORY_EDITOR_API std::string CachePath();

	private:
		ProjectSpace(const std::string& path);
		virtual ~ProjectSpace();

		void Open();
		void Close();

	private:
		static ProjectSpace* m_pCurrentProject;
		static std::mutex m_ProjectLock;
		std::string m_ProjectFilePath;
		std::string m_ProjectRootPath;
		std::string m_CachePath;
		std::string m_ProjectName;
	};
}
