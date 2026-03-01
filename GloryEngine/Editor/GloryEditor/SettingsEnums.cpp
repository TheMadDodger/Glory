#include "SettingsEnums.h"

#include <IEngine.h>

namespace Glory::Editor
{
    void RegisterSettingsEnums(IEngine* pEngine)
    {
        pEngine->Reflection().RegisterEnum<PackageScenes>();
    }
}
