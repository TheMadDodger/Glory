#pragma once
#include "GloryEditor.h"

#include <Reflection.h>

REFLECTABLE_ENUM_NS(Glory::Editor, PackageScenes, All, Opened, List)

namespace Glory
{
	class Engine;
}

namespace Glory::Editor
{
	GLORY_EDITOR_API void RegisterSettingsEnums(Engine* pEngine);
}