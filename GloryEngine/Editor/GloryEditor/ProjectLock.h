#pragma once
#include "GloryEditor.h"

#include <string>
#include <fstream>
#include <filesystem>

namespace Glory::Editor
{
	class ProjectLock
	{
	public:
		GLORY_EDITOR_API ProjectLock(const std::string& path);
		GLORY_EDITOR_API virtual ~ProjectLock();

		GLORY_EDITOR_API bool Lock();
		GLORY_EDITOR_API void Unlock();
		GLORY_EDITOR_API bool CanLock();

	private:
		const std::string m_ProjectPath;
		std::filesystem::path m_ProjectLockPath;
		std::ofstream m_FileStream;
	};
}
