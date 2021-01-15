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
		virtual void Initialize_Internal() override;
		virtual void Cleanup_Internal() override;
		virtual void Update_Internal() override;
		virtual void Draw_Internal() override;
    };
}