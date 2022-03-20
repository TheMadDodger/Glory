#pragma once
#include <Engine.h>

namespace Glory::Editor
{
	class EditorSettings
	{
	public:
		EditorSettings(std::filesystem::path editorSettingsPath = "./EditorSettings.yaml");
		virtual ~EditorSettings();
		void Load(Engine* pEngine);
		void Save(Engine* pEngine);


	private:
		void LoadSettingsFile(Engine* pEngine);
		void LoadDefaultSettings(Engine* pEngine);

	private:
		std::filesystem::path m_EditorSettingsPath;
	};
}
