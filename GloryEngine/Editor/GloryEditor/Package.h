#pragma once
#include "GloryEditor.h"

namespace std::filesystem
{
	class path;
}

namespace Glory
{
	class GScene;

	GLORY_EDITOR_API void PackageScene(GScene* pScene, const std::filesystem::path& path);
}