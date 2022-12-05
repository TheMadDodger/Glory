#include "pch.h"
#include "EditorManager.h"
#include <Windows.h>

namespace Glory::EditorLauncher
{
    std::vector<EditorInfo> EditorManager::m_InstalledEditors;

    void EditorManager::GetInstalledEditors()
    {
        std::filesystem::path searchPath = "./Editor";
        if (!std::filesystem::exists(searchPath)) return;
        for (const auto& entry : std::filesystem::directory_iterator(searchPath))
        {
            if (!entry.is_directory()) continue;
            std::filesystem::path editorPath = entry.path();
            EditorInfo info;
            info.RootPath = editorPath;
            editorPath.append("GloryEditor.dll");
            if (!std::filesystem::exists(editorPath)) continue;
            if (!GetEditorInfo(editorPath, info)) continue;
            m_InstalledEditors.push_back(info);
        }
    }

    size_t EditorManager::EditorCount()
    {
        return m_InstalledEditors.size();
    }

    const EditorInfo& EditorManager::GetEditorInfo(size_t index)
    {
        return m_InstalledEditors[index];
    }

    const std::filesystem::path& EditorManager::GetEditorLocation(const Version& version)
    {
        auto it = std::find_if(m_InstalledEditors.begin(), m_InstalledEditors.end(), [version](const EditorInfo& editorInfo) { return editorInfo.Version.HardCompare(version) == 0; });
        if (it == m_InstalledEditors.end()) return "";
        return it->RootPath;
    }

    bool EditorManager::IsInstalled(const Version& version)
    {
        auto it = std::find_if(m_InstalledEditors.begin(), m_InstalledEditors.end(), [version](const EditorInfo& editorInfo) { return editorInfo.Version.HardCompare(version) == 0; });
        return it != m_InstalledEditors.end();
    }

    bool EditorManager::GetEditorInfo(std::filesystem::path editorDLL, EditorInfo& info)
    {
        HMODULE lib = LoadLibrary(editorDLL.wstring().c_str());
        if (lib == NULL) return false;
        GetVersionProc versionProc = (GetVersionProc)GetProcAddress(lib, "GetVersion");
        if (versionProc == NULL)
        {
            FreeLibrary(lib);
            return false;
        }
        info.Version = versionProc();
        FreeLibrary(lib);
        return true;
    }

    EditorManager::EditorManager()
    {
    }

    EditorManager::~EditorManager()
    {
    }
}
