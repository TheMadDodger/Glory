#include "EditorModuleLoopHandler.h"
#include <Engine.h>

namespace Glory::Editor
{
    bool EditorModuleLoopHandler::HandleModuleLoop(Module* pModule)
    {
        const ModuleMetaData& metaData = pModule->GetMetaData();
        if (metaData.Type() != ModuleType::MT_SceneManagement) return false;

        // Update should be handled by the editor
        // In edit mode only editor essential components need to be updated
        // In play mode everything is updated

        pModule->GetEngine()->CallModuleDraw(pModule);
        return true;
    }
}
