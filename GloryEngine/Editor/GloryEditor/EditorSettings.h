#pragma once
#include <Engine.h>
#include <NodeRef.h>
#include <GloryEditor.h>

namespace Glory::Editor
{
	class EditorSettings
	{
	public:
		EditorSettings(std::filesystem::path editorSettingsPath = "./EditorSettings.yaml");
		virtual ~EditorSettings();
		void Load(Engine* pEngine);
		void Save(Engine* pEngine);

		GLORY_EDITOR_API NodeValueRef operator[](const std::filesystem::path& path);

	private:
		void LoadSettingsFile(Engine* pEngine);
		void LoadDefaultSettings(Engine* pEngine);

	private:
		YAMLFileRef m_YAMLFile;
	};
}
