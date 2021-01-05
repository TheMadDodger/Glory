#include "WindowModule.h"

namespace Glory
{
    WindowModule::WindowModule() : m_pMainWindow(nullptr), m_pWindows(std::vector<Window*>())
    {
    }

    WindowModule::~WindowModule()
    {
    }

    Window* WindowModule::CreateNewWindow(const WindowCreateInfo& createInfo)
    {
        Window* pWindow = CreateWindow_Internal(createInfo);
        pWindow->Open();
        m_pWindows.push_back(pWindow);
        return pWindow;
    }

    void WindowModule::Initialize()
    {
        Initialize_Internal();

        // TEMPORARY
        WindowCreateInfo createInfo;
        createInfo.WindowName = "Glory Game";
        createInfo.Width = 1280;
        createInfo.Height = 720;
        m_pMainWindow = CreateNewWindow(createInfo);
    }

    void WindowModule::Cleanup()
    {
        for (size_t i = 0; i < m_pWindows.size(); i++)
        {
            m_pWindows[i]->Close();
            delete m_pWindows[i];
        }
        m_pWindows.clear();

        Cleanup_Internal();
    }

    void WindowModule::Update()
    {
        for (size_t i = 0; i < m_pWindows.size(); i++)
        {
            m_pWindows[i]->PollEvents();
        }
        Update_Internal();
    }

    void WindowModule::Draw()
    {
        Draw_Internal();
    }
}