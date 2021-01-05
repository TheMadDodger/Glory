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
		virtual ~WindowModule();

	public: // Module functions
		Window* CreateNewWindow(const WindowCreateInfo& createInfo);

	protected: // Internal functions
		virtual Window* CreateWindow_Internal(const WindowCreateInfo& createInfo) = 0;

	protected:
		virtual void Initialize_Internal() = 0;
		virtual void Cleanup_Internal() = 0;
		virtual void Update_Internal() = 0;
		virtual void Draw_Internal() = 0;

	private:
		virtual void Initialize();
		virtual void Cleanup();
		virtual void Update();
		virtual void Draw();

	private: // Memory stuff
		std::vector<Window*> m_pWindows;
		Window* m_pMainWindow;
    };
}
