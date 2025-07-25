#include "SDLWindowModule.h"
#include "SDLWindow.h"
#include "SDLWindowExceptions.h"

#include <Engine.h>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(SDLWindowModule);

	SDLWindowModule::SDLWindowModule()
	{
	}
	SDLWindowModule::~SDLWindowModule() {}

	void SDLWindowModule::GetCurrentScreenResolution(uint32_t& width, uint32_t& height)
	{
		SDL_DisplayMode DM;
		SDL_GetCurrentDisplayMode(0, &DM);
		width = DM.w;
		height = DM.h;
	}

	std::filesystem::path SDLWindowModule::GetPrefPath() const
	{
		char* prefPathStr = SDL_GetPrefPath(m_pEngine->Organization().data(), m_pEngine->AppName().data());
		std::filesystem::path path = prefPathStr;
		SDL_free(prefPathStr);
		return path;
	}

	Window* SDLWindowModule::CreateWindow_Internal(const WindowCreateInfo& createInfo)
	{
		return new SDLWindow(createInfo, this);
	}

	void SDLWindowModule::OpenMessageBox(const std::string& message)
	{
		const SDL_MessageBoxButtonData buttons[] = {
				{ /* .flags, .buttonid, .text */        0, 0, "OK" },
		};
		const SDL_MessageBoxColorScheme colorScheme =
		{
			{ /* .colors (.r, .g, .b) */
			  /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
				{ 255, 255, 255 },
				/* [SDL_MESSAGEBOX_COLOR_TEXT] */
				{ 0, 0, 0 },
				/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
				{ 0, 0, 0 },
				/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
				{ 255, 255, 255 },
				/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
				{ 128, 128, 128 }
			}
		};
		const SDL_MessageBoxData messageboxdata = {
			SDL_MESSAGEBOX_ERROR, /* .flags */
			NULL, /* .window */
			"ERROR", /* .title */
			message.data(), /* .message */
			SDL_arraysize(buttons), /* .numbuttons */
			buttons, /* .buttons */
			&colorScheme /* .colorScheme */
		};
		int buttonid;
		if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
			SDL_Log("error displaying message box");
		}
	}

	void SDLWindowModule::OnInitialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
			throw new SDLErrorException(SDL_GetError());
	}

	void SDLWindowModule::OnCleanup()
	{
		SDL_Quit();
	}
}