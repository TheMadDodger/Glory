#include "GlorySDLImage.h"
#include "SDLImageLoaderModule.h"

GLORY_API Glory::Module* LoadModule()
{
	return new Glory::SDLImageLoaderModule();
}
