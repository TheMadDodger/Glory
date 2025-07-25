#pragma once
#include "Module.h"

#include <vector>

namespace Glory
{
	class Window;
	struct WindowCreateInfo;

    class WindowModule : public Module
    {
	public:
		WindowModule();
		virtual ~WindowModule();

		Window* GetMainWindow();

		virtual const std::type_info& GetModuleType() override;

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
    };
}
