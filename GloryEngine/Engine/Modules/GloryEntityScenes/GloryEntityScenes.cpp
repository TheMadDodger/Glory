#include "GloryEntityScenes.h"
#include "EntitySceneScenesModule.h"

GLORY_API Glory::Module* OnLoadModule(Glory::GloryContext* pContext)
{
    Glory::GloryContext::SetContext(pContext);
    return new Glory::EntitySceneScenesModule();
}
