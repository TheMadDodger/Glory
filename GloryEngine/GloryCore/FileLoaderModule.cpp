#include "FileLoaderModule.h"
#include <fstream>
#include "Debug.h"

namespace Glory
{
	FileLoaderModule::FileLoaderModule()
	{
	}

	FileLoaderModule::~FileLoaderModule()
	{
	}

	const std::type_info& FileLoaderModule::GetModuleType()
	{
		return typeid(FileLoaderModule);
	}

	FileData* FileLoaderModule::LoadResource(const std::string& path, const FileImportSettings& importSettings)
	{
		FileData* pFile = new FileData();
		if (!ReadFile(path, pFile->m_Data, importSettings))
		{
			delete pFile;
			return nullptr;
		}
		return pFile;
	}

	FileData* FileLoaderModule::LoadResource(const void* buffer, size_t length, const FileImportSettings& importSettings)
	{
		FileData* pFile = new FileData();
		pFile->m_Data.resize(length);
		memcpy(&pFile->m_Data[0], buffer, length);
		return pFile;
	}

	bool FileLoaderModule::ReadFile(const std::string& path, std::vector<char>& buffer, const FileImportSettings& importSettings)
	{
		//auto f = std::ios::ate | std::ios::binary;

		std::ifstream file(path, importSettings.Flags);

		if (!file.is_open())
		{
			Debug::LogError("Could not open file: " + path);
			return false;
			//throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		buffer.resize(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		if (importSettings.AddNullTerminateAtEnd) buffer.push_back('\0');
		file.close();
		return true;
	}
}
