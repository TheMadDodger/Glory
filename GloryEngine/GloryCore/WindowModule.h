#pragma once
#include "Module.h"
#include "Window.h"
#include <vector>

namespace Glory
{
    class WindowModule : public Module
    {
	public:
		WindowModule(const WindowCreateInfo& mainWindowCreateInfo);
		virtual ~WindowModule();

		Window* GetMainWindow();

		virtual const std::type_info& GetModuleType() override;

	public: // Module functions
		Window* CreateNewWindow(const WindowCreateInfo& createInfo);
		virtual void OpenMessageBox(const std::string& message);

	protected: // Internal functions
		virtual Window* CreateWindow_Internal(const WindowCreateInfo& createInfo) = 0;

	protected:
		virtual void OnInitialize() = 0;
		virtual void OnCleanup() = 0;
		virtual void OnMainUpdate() = 0;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void MainUpdate();

	private: // Memory stuff
		friend class MainThread;
		std::vector<Window*> m_pWindows;
		Window* m_pMainWindow;
		WindowCreateInfo m_MainWindowCreateInfo;
    };
}
