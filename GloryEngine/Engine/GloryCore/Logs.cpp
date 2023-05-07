#include "Logs.h"
#include <fstream>
#include <filesystem>

namespace Glory
{
	const std::string LogsPath = "./Logs";
	const std::string LogPath = "./Logs/Editor.log";
	const std::string LogBackupPath = "./Logs/Editor.log.bak";

	void Logs::Initialize()
	{
		if (!std::filesystem::exists(LogsPath))
			std::filesystem::create_directory(LogsPath);

		if (std::filesystem::exists(LogPath))
			std::filesystem::copy_file(LogPath, LogBackupPath, std::filesystem::copy_options::overwrite_existing);

		std::filesystem::remove(LogPath);
	}

	void Logs::Write(const std::string& line)
	{
		std::ofstream log(LogPath, std::ios_base::app | std::ios_base::out);
		log << line << std::endl;
	}
}
