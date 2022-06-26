#include "GlorySDLImage.h"
#include "SDLImageLoaderModule.h"

GLORY_API Glory::Module* LoadModule(Glory::GloryContext* pContext)
{
	Glory::GloryContext::SetContext(pContext);
	return new Glory::SDLImageLoaderModule();
}
