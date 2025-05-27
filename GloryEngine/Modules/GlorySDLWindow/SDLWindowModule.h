#pragma once
#include <WindowModule.h>

namespace Glory
{
    class SDLWindowModule : public WindowModule
    {
	public:
		SDLWindowModule();
		SDLWindowModule(const WindowCreateInfo& mainWindowCreateInfo);
		virtual ~SDLWindowModule();

		virtual void GetCurrentScreenResolution(uint32_t& width, uint32_t& height) override;

		virtual std::filesystem::path GetPrefPath() const override;

		GLORY_MODULE_VERSION_H(0,3,0);

	protected: // Internal functions
		virtual Window* CreateWindow_Internal(const WindowCreateInfo& createInfo) override;
		virtual void OpenMessageBox(const std::string& message) override;

	protected:
		virtual void OnInitialize() override;
		virtual void OnCleanup() override;
    };
}
