#include "SDLWindowModule.h"
#include "SDLWindow.h"
#include "SDLWindowExceptions.h"

namespace Glory
{
	SDLWindowModule::SDLWindowModule(const WindowCreateInfo& mainWindowCreateInfo) : WindowModule(mainWindowCreateInfo) {}
	SDLWindowModule::~SDLWindowModule() {}

	Window* SDLWindowModule::CreateWindow_Internal(const WindowCreateInfo& createInfo)
	{
		return new SDLWindow(createInfo);
	}

	void SDLWindowModule::Initialize_Internal()
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
			throw new SDLErrorException(SDL_GetError());
	}

	void SDLWindowModule::Cleanup_Internal()
	{
	}

	void SDLWindowModule::Update_Internal()
	{
	}

	void SDLWindowModule::Draw_Internal()
	{
	}
}