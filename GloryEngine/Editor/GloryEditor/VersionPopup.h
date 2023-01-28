#pragma once
#include "EditorWindow.h"

namespace Glory::Editor
{
    class VersionPopup
    {
    public:
        GLORY_EDITOR_API static void Open(const Glory::Version& latestVersion);

    private:
        static void Draw();

    private:
        friend class MainEditor;
    };
}
