#include "SettingsEnums.h"

#include <Engine.h>

namespace Glory::Editor
{
    void RegisterSettingsEnums(Engine* pEngine)
    {
        pEngine->Reflection().RegisterEnum<PackageScenes>();
    }
}
