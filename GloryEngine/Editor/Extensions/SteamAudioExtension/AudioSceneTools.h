#pragma once
#include "GloryEditor.h"

#include <AudioScene.h>

namespace Glory
{
	class GScene;
	class IEngine;
}

namespace Glory::Editor
{
	GLORY_EDITOR_API bool GenerateAudioScene(IEngine* pEngine, GScene* pScene, const SoundMaterial* defaultMaterial, AudioScene& audioScene);
}
