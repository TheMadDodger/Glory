#include "GlorySDLWindow.h"
#include "SDLWindowModule.h"

GLORY_API Glory::Module* LoadModule()
{
	return new Glory::SDLWindowModule();
}
