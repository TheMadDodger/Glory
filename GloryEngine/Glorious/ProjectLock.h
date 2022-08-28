#pragma once
#include <string>
#include <fstream>
#include <filesystem>

namespace Glory::Editor
{
	class ProjectLock
	{
	public:
		ProjectLock(const std::string& path);
		virtual ~ProjectLock();

		bool Lock();
		void Unlock();
		bool CanLock();

	private:
		const std::string m_ProjectPath;
		std::filesystem::path m_ProjectLockPath;
		std::ofstream m_FileStream;
	};
}
