#pragma once
#include <Versioning.h>

typedef Glory::Version (__cdecl* GetVersionProc)();

namespace Glory::EditorLauncher
{
	struct EditorInfo
	{
		Glory::Version Version;
		std::filesystem::path RootPath;
	};

	class EditorManager
	{
	public:
		static void GetInstalledEditors();
		static size_t EditorCount();
		static const EditorInfo& GetEditorInfo(size_t index);
		static const std::filesystem::path& GetEditorLocation(const Version& version);

		static bool IsInstalled(const Version& version);

	private:
		static bool GetEditorInfo(std::filesystem::path editorDLL, EditorInfo& info);

	private:
		EditorManager();
		virtual ~EditorManager();

	private:
		static std::vector<EditorInfo> m_InstalledEditors;
	};
}
