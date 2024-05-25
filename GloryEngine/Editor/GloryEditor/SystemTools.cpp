#include "SystemTools.h"

namespace Glory::Editor
{
    bool FindVisualStudio(std::filesystem::path& out)
    {
		const std::filesystem::path x64Path = "C:\\Program Files\\Microsoft Visual Studio";
		const std::filesystem::path x86Path = "C:\\Program Files (x86)\\Microsoft Visual Studio";
		return FindVisualStudio(x64Path, out) || FindVisualStudio(x86Path, out);
    }

    bool FindVisualStudio(const std::filesystem::path& path, std::filesystem::path& out)
    {
		if (!std::filesystem::exists(path)) return false;
		for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(path))
		{
			if (!entry.is_directory()) continue;
			const std::filesystem::path versionPath = entry.path();
			std::filesystem::path editionPath = versionPath;
			editionPath.append("Enterprise");
			if (FindMSBuildInVSPath(std::filesystem::path(editionPath), out))
			{
				out = editionPath;
				return true;
			}

			editionPath = versionPath;
			editionPath.append("Professional");
			if (FindMSBuildInVSPath(std::filesystem::path(editionPath), out))
			{
				out = editionPath;
				return true;
			}

			editionPath = versionPath;
			editionPath.append("Community");
			if (FindMSBuildInVSPath(std::filesystem::path(editionPath), out))
			{
				out = editionPath;
				return true;
			}
		}

		return false;
    }

	bool FindMSBuildInVSPath(std::filesystem::path& path, std::filesystem::path& out)
	{
		path.append("MSBuild\\Current\\Bin\\MSBuild.exe");
		if (std::filesystem::exists(path))
		{
			out = path;
			return true;
		}
		return false;
	}
}
