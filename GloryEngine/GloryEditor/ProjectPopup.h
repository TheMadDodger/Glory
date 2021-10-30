#pragma once
#include <string>
#include <vector>

namespace Glory::Editor
{
	struct Project
	{
		std::string Name;
		std::string Path;
	};

	class ProjectPopup
	{
	public:
		ProjectPopup();
		virtual ~ProjectPopup();

		void Initialize();
		void Load();
		void Save();

		void Open();
		void OnGui();

		void OnHubGui();
		void OnProjectNotFoundGui();
		void OnFileDialogPopupGui();
		void OnNewProjectPopupGui();

	private:
		bool m_Open;
		bool m_OpenErrorPopup;
		bool m_OpenFileDialogPopup;
		bool m_OpenNewProjectPopup;

		std::filesystem::path m_BrowsingPath;

		std::vector<Project> m_Projects;
		size_t m_RemovingIndex;
		float m_Width;
		float m_Height;

		char m_PathText[100];
		char m_ProjectNameText[100] = "New Project";

		std::string m_DefaultProjectsFolder = "%userprofile%\\documents\\Glorious\\Projects";
	};
}
