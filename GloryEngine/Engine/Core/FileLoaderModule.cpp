#include "FileLoaderModule.h"
#include "Debug.h"
#include "EngineProfiler.h"
#include <fstream>

namespace Glory
{
	FileLoaderModule::FileLoaderModule() : ResourceLoaderModule(".txt")
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
		Profiler::BeginSample("FileLoaderModule::LoadResource(path)");
		FileData* pFile = new FileData();
		if (!ReadFile(path, pFile->m_Data, importSettings))
		{
			delete pFile;
			Profiler::EndSample();
			return nullptr;
		}
		Profiler::EndSample();
		return pFile;
	}

	FileData* FileLoaderModule::LoadResource(const void* buffer, size_t length, const FileImportSettings& importSettings)
	{
		Profiler::BeginSample("FileLoaderModule::LoadResource(buffer)");
		FileData* pFile = new FileData();
		pFile->m_Data.resize(length);
		memcpy(&pFile->m_Data[0], buffer, length);
		Profiler::EndSample();
		return pFile;
	}

	void FileLoaderModule::SaveResource(const std::string& path, FileData* pResource)
	{
		Profiler::BeginSample("FileLoaderModule::SaveResource");
		std::ofstream file(path);

		if (!file.is_open())
		{
			Debug::LogError("Could not open file: " + path);
			Profiler::EndSample();
			return;
		}

		file.write(pResource->m_Data.data(), pResource->Size());
		file.close();
		Profiler::EndSample();
	}

	bool FileLoaderModule::ReadFile(const std::string& path, std::vector<char>& buffer, const FileImportSettings& importSettings)
	{
		Profiler::BeginSample("FileLoaderModule::ReadFile");
		//auto f = std::ios::ate | std::ios::binary;

		std::ifstream file(path, importSettings.Flags);

		if (!file.is_open())
		{
			Debug::LogError("Could not open file: " + path);
			Profiler::EndSample();
			return false;
		}

		size_t fileSize = (size_t)file.tellg();
		buffer.resize(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		if (importSettings.AddNullTerminateAtEnd) buffer.push_back('\0');
		file.close();
		Profiler::EndSample();
		return true;
	}

	FileImportSettings FileLoaderModule::ReadImportSettings_Internal(YAML::Node& node)
	{
		FileImportSettings importSettings;
		YAML::Node nextNode;
		YAML_READ(node, nextNode, Flags, importSettings.Flags, int);
		YAML_READ(node, nextNode, AddNullTerminateAtEnd, importSettings.AddNullTerminateAtEnd, bool);
		return importSettings;
	}

	void FileLoaderModule::WriteImportSettings_Internal(const FileImportSettings& importSettings, YAML::Emitter& out)
	{
		YAML_WRITE(out, Flags, importSettings.Flags);
		YAML_WRITE(out, AddNullTerminateAtEnd, importSettings.AddNullTerminateAtEnd);
	}

	FileImportSettings::FileImportSettings()
		: Flags(std::ios::ate), AddNullTerminateAtEnd(false) {}

	FileImportSettings::FileImportSettings(const std::string& extension)
		: Flags(std::ios::ate), AddNullTerminateAtEnd(false), ImportSettings(extension) {}
}
