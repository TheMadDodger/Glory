#pragma once
#include <WindowModule.h>

namespace Glory
{
    class SDLWindowModule : public WindowModule
    {
	protected: // Internal functions
		virtual Window* CreateWindow_Internal(const WindowCreateInfo& createInfo) override;

	protected:
		virtual void Initialize_Internal() override;
		virtual void Cleanup_Internal() override;
		virtual void Update_Internal() override;
		virtual void Draw_Internal() override;
    };
}