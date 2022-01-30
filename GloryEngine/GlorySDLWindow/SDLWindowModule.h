#pragma once
#include <WindowModule.h>

namespace Glory
{
    class SDLWindowModule : public WindowModule
    {
	public:
		SDLWindowModule(const WindowCreateInfo& mainWindowCreateInfo);
		virtual ~SDLWindowModule();

	protected: // Internal functions
		virtual Window* CreateWindow_Internal(const WindowCreateInfo& createInfo) override;
		virtual void OpenMessageBox(const std::string& message) override;

	protected:
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
    };
}