#pragma once
#include <string>
#include <vector>
#include "GloryEditor.h"

namespace std::filesystem
{
	class path;
}


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
		GLORY_EDITOR_API ProjectPopup();
		virtual GLORY_EDITOR_API ~ProjectPopup();

		GLORY_EDITOR_API void Initialize();
		GLORY_EDITOR_API void Load();
		GLORY_EDITOR_API void Save();

		GLORY_EDITOR_API void Open();
		GLORY_EDITOR_API void OnGui();

		GLORY_EDITOR_API void OnHubGui();
		GLORY_EDITOR_API void OnProjectNotFoundGui();
		GLORY_EDITOR_API void OnFileDialogPopupGui();
		GLORY_EDITOR_API void OnNewProjectPopupGui();

	private:
		bool m_Open;
		bool m_OpenErrorPopup;
		bool m_OpenFileDialogPopup;
		bool m_OpenNewProjectPopup;

		std::string m_BrowsingPath;

		std::vector<Project> m_Projects;
		size_t m_RemovingIndex;
		float m_Width;
		float m_Height;

		char m_PathText[100];
		char m_ProjectNameText[100] = "New Project";

		std::string m_DefaultProjectsFolder = "%userprofile%\\documents\\Glorious\\Projects";
	};
}
