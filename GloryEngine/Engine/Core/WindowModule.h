#pragma once
#include "Module.h"
#include "Window.h"

#include <vector>

namespace Glory
{
    class WindowModule : public Module
    {
	public:
		WindowModule();
		WindowModule(const WindowCreateInfo& mainWindowCreateInfo);
		virtual ~WindowModule();

		Window* GetMainWindow();

		virtual const std::type_info& GetModuleType() override;

		WindowCreateInfo* GetMainWindowCreateInfo();
		void SetMainWindowCreateInfo(const WindowCreateInfo& mainWindowCreateInfo);

		virtual void GetCurrentScreenResolution(uint32_t& width, uint32_t& height) = 0;

		virtual std::filesystem::path GetPrefPath() const = 0;

	public: // Module functions
		Window* CreateNewWindow(WindowCreateInfo& createInfo);
		virtual void OpenMessageBox(const std::string& message);

	protected: // Internal functions
		virtual Window* CreateWindow_Internal(const WindowCreateInfo& createInfo) = 0;

	protected:
		virtual void OnInitialize() = 0;
		virtual void OnCleanup() = 0;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;
		virtual void PollEvents();

	private: // Memory stuff
		friend class MainThread;
		friend class Engine;
		std::vector<Window*> m_pWindows;
		Window* m_pMainWindow;
		WindowCreateInfo m_MainWindowCreateInfo;
    };
}
