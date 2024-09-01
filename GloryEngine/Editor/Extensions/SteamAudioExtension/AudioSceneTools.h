#pragma once
#include "GloryEditor.h"

#include <AudioScene.h>

namespace Glory
{
	class GScene;
	class Engine;
}

namespace Glory::Editor
{
	GLORY_EDITOR_API bool GenerateAudioScene(Engine* pEngine, GScene* pScene, const SoundMaterial* defaultMaterial, AudioScene& audioScene);
}
