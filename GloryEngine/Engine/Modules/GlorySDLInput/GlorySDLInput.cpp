#include <GloryContext.h>
#include "SDLInputModule.h"
#include "GlorySDLInput.h"

GLORY_API Glory::Module* OnLoadModule(Glory::GloryContext* pContext)
{
    Glory::GloryContext::SetContext(pContext);
    return new Glory::SDLInputModule();
}
