#include "stdafx.h"
#include "GloryCore.h"
#include "CoreExceptions.h"

namespace Glory
{
	void GloryCore::Initialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
			throw new SDLErrorException(SDL_GetError());
	}

	void GloryCore::Destroy()
	{
		SDL_Quit();
	}

	GloryCore::~GloryCore()
	{
	}
}