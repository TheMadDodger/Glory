#include "WindowModule.h"

namespace Glory
{
    WindowModule::WindowModule(const WindowCreateInfo& mainWindowCreateInfo) : m_pMainWindow(nullptr), m_pWindows(std::vector<Window*>()), m_MainWindowCreateInfo(mainWindowCreateInfo)
    {
    }

    WindowModule::~WindowModule()
    {
    }

    Window* WindowModule::GetMainWindow()
    {
        return m_pMainWindow;
    }

    const std::type_info& WindowModule::GetModuleType()
    {
        return typeid(WindowModule);
    }

    WindowCreateInfo* WindowModule::GetMainWindowCreateInfo()
    {
        return &m_MainWindowCreateInfo;
    }

    Window* WindowModule::CreateNewWindow(const WindowCreateInfo& createInfo)
    {
        Window* pWindow = CreateWindow_Internal(createInfo);
        pWindow->Open();
        m_pWindows.push_back(pWindow);
        return pWindow;
    }

    void WindowModule::OpenMessageBox(const std::string&) {}

    void WindowModule::Initialize()
    {
        OnInitialize();
        m_pMainWindow = CreateNewWindow(m_MainWindowCreateInfo);
    }

    void WindowModule::Cleanup()
    {
        for (size_t i = 0; i < m_pWindows.size(); i++)
        {
            m_pWindows[i]->Close();
            delete m_pWindows[i];
        }
        m_pWindows.clear();
        m_pMainWindow = nullptr;

        OnCleanup();
    }

    void WindowModule::PollEvents()
    {
        for (size_t i = 0; i < m_pWindows.size(); i++)
        {
            m_pWindows[i]->PollEvents();
        }
    }
}