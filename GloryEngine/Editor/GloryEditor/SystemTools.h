#pragma once
#include "GloryEditor.h"

#include <filesystem>

namespace Glory::Editor
{
	GLORY_EDITOR_API bool FindVisualStudio(std::filesystem::path& out);
	GLORY_EDITOR_API bool FindVisualStudio(const std::filesystem::path& path, std::filesystem::path& out);
	GLORY_EDITOR_API bool FindMSBuildInVSPath(std::filesystem::path& path, std::filesystem::path& out);
}
