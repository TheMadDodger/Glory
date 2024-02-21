#pragma once
#include "GloryEditor.h"

#include <UUID.h>

namespace std::filesystem
{
	class path;
}

namespace Glory
{
	class GScene;
	class Engine;
	class AssetArchive;

	namespace Utils
	{
		struct YAMLFileRef;
	}
}

namespace Glory::Editor
{
	GLORY_EDITOR_API void Package(Engine* pEngine);
	GLORY_EDITOR_API void ScanSceneFileForAssets(Engine* pEngine, Utils::YAMLFileRef& file, std::vector<UUID>& assets);
	GLORY_EDITOR_API void PackageScene(GScene* pScene, const std::filesystem::path& path);
}