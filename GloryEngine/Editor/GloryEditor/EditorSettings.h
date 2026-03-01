#pragma once
#include <IEngine.h>
#include <NodeRef.h>
#include <GloryEditor.h>

namespace Glory::Editor
{
	class EditorSettings
	{
	public:
		EditorSettings(std::filesystem::path editorSettingsPath = "./EditorSettings.yaml");
		virtual ~EditorSettings();
		void Load(IEngine* pEngine);
		void Save(IEngine* pEngine);

		GLORY_EDITOR_API Utils::NodeValueRef operator[](const std::filesystem::path& path);

	private:
		void LoadSettingsFile(IEngine* pEngine);
		void LoadDefaultSettings(IEngine* pEngine);

	private:
		Utils::YAMLFileRef m_YAMLFile;
	};
}
