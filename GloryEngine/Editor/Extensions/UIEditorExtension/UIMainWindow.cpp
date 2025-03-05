#include "UIMainWindow.h"

namespace Glory::Editor
{
    std::string_view UIMainWindow::Name()
    {
        return "UI Editing";
    }

    void UIMainWindow::OnGui(float height)
    {
        Dockspace(height);
    }

    void UIMainWindow::Initialize()
    {
    }
}
