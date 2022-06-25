#include "GloryEntityScenes.h"
#include "EntitySceneScenesModule.h"

GLORY_API Glory::Module* LoadModule()
{
    return new Glory::EntitySceneScenesModule();
}
