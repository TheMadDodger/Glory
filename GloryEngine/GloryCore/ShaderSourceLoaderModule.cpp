#include "ShaderSourceLoaderModule.h"
#include "Debug.h"
#include "EngineProfiler.h"
#include "ShaderManager.h"
#include <fstream>

namespace Glory
{
	std::map<std::string, ShaderType> ShaderSourceLoaderModule::m_ShaderTypes = {
		{ "compute", ShaderType::ST_Compute },
		{ "comp", ShaderType::ST_Compute },
		{ "fragment", ShaderType::ST_Fragment },
		{ "frag", ShaderType::ST_Fragment },
		{ "geometry", ShaderType::ST_Geomtery },
		{ "geom", ShaderType::ST_Geomtery },
		{ "tesscontrol", ShaderType::ST_TessControl },
		{ "tc", ShaderType::ST_TessControl },
		{ "tesselationcontrol", ShaderType::ST_TessControl },
		{ "teseval", ShaderType::ST_TessEval },
		{ "te", ShaderType::ST_TessEval },
		{ "tesselationevaluation", ShaderType::ST_TessEval },
		{ "vertex", ShaderType::ST_Vertex },
		{ "vert", ShaderType::ST_Vertex },
	};

	ShaderSourceLoaderModule::ShaderSourceLoaderModule() : ResourceLoaderModule("shader")
	{
		m_SymbolCallbacks["type"] = [&](const std::string& path, ShaderSourceData* pShaderSource, const std::string& argument)
		{
			return GetShaderTypeFromString(argument, pShaderSource->m_ShaderType);
		};
	}

	ShaderSourceLoaderModule::~ShaderSourceLoaderModule()
	{
	}

	const std::type_info& ShaderSourceLoaderModule::GetModuleType()
	{
		return typeid(ShaderSourceLoaderModule);
	}

	ShaderSourceData* ShaderSourceLoaderModule::LoadResource(const std::string& path, const ShaderSourceImportSettings& importSettings)
	{
		Profiler::BeginSample("FileLoaderModule::LoadResource(path)");
		ShaderSourceData* pFile = new ShaderSourceData();
		if (!ReadFile(path, pFile, importSettings))
		{
			delete pFile;
			Profiler::EndSample();
			return nullptr;
		}
		Profiler::EndSample();
		return pFile;
	}

	ShaderSourceData* ShaderSourceLoaderModule::LoadResource(const void* buffer, size_t length, const ShaderSourceImportSettings& importSettings)
	{
		// Should never be called because this shader loader is not meant for game builds
		Profiler::BeginSample("FileLoaderModule::LoadResource(buffer)");
		ShaderSourceData* pFile = new ShaderSourceData();
		pFile->m_ProcessedSource.resize(length);
		memcpy(&pFile->m_ProcessedSource[0], buffer, length);
		Profiler::EndSample();
		return pFile;
	}

	bool ShaderSourceLoaderModule::ReadFile(const std::string& path, ShaderSourceData* pShaderSource, const ShaderSourceImportSettings& importSettings)
	{
		Profiler::BeginSample("FileLoaderModule::ReadFile");
		std::ifstream file(path, std::ios::binary);

		if (!file.is_open())
		{
			Debug::LogError("Could not open file: " + path);
			Profiler::EndSample();
			return false;
		}

		file.seekg(0, std::ios_base::end);
		size_t fileSize = (size_t)file.tellg();
		pShaderSource->m_OriginalSource.resize(fileSize);
		file.seekg(0, std::ios_base::beg);
		file.read(pShaderSource->m_OriginalSource.data(), fileSize);
		file.close();


		std::string originalSourceString(pShaderSource->m_OriginalSource.begin(), pShaderSource->m_OriginalSource.end());
		std::istringstream stream(originalSourceString);
		std::string line;

		for (std::string line; std::getline(stream, line); )
		{
			ProcessLine(path, pShaderSource, line);
		}

		Profiler::EndSample();

		return true;
	}

	ShaderSourceImportSettings ShaderSourceLoaderModule::ReadImportSettings_Internal(YAML::Node& node)
	{
		ShaderSourceImportSettings importSettings;
		YAML::Node nextNode;
		//YAML_READ(node, nextNode, Flags, importSettings.Flags, int);
		//YAML_READ(node, nextNode, AddNullTerminateAtEnd, importSettings.AddNullTerminateAtEnd, bool);
		return importSettings;
	}

	void ShaderSourceLoaderModule::WriteImportSettings_Internal(const ShaderSourceImportSettings& importSettings, YAML::Emitter& out)
	{
		//YAML_WRITE(out, Flags, importSettings.Flags);
		//YAML_WRITE(out, AddNullTerminateAtEnd, importSettings.AddNullTerminateAtEnd);
	}

	void ShaderSourceLoaderModule::ProcessLine(const std::string& path, ShaderSourceData* pShaderSource, const std::string& line)
	{
		if (line[0] != '#')
		{
			AppendLine(line, pShaderSource->m_ProcessedSource);
			return;
		}

		size_t spaceIndex = line.find(' ');
		if (spaceIndex == std::string::npos) spaceIndex == line.length() - 1;
		std::string symbol = line.substr(1, spaceIndex - 1);
		if (symbol[symbol.length() - 1] == '\r') symbol = symbol.substr(0, symbol.length() - 1);

		std::string argument = line.substr(spaceIndex + 1);
		if (argument[argument.length() - 1] == '\r') argument = argument.substr(0, argument.length() - 1);

		if (!ProcessSymbol(path, pShaderSource, symbol, argument)) AppendLine(line, pShaderSource->m_ProcessedSource);
	}

	void ShaderSourceLoaderModule::AppendLine(const std::string& line, std::vector<char>& buffer)
	{
		for (size_t i = 0; i < line.length(); i++) buffer.push_back(line[i]);
		buffer.push_back('\n');
	}

	bool ShaderSourceLoaderModule::GetShaderTypeFromString(const std::string& typeString, ShaderType& shaderType)
	{
		if (m_ShaderTypes.find(typeString) == m_ShaderTypes.end()) return false;
		shaderType = m_ShaderTypes[typeString];
		return true;
	}

	bool ShaderSourceLoaderModule::ProcessSymbol(const std::string& path, ShaderSourceData* pShaderSource, const std::string& symbol, const std::string& argument)
	{
		if (m_SymbolCallbacks.find(symbol) == m_SymbolCallbacks.end()) return false;
		return m_SymbolCallbacks[symbol](path, pShaderSource, argument);
	}

	ShaderSourceImportSettings::ShaderSourceImportSettings() : ImportSettings(".shader")
	{
	}

	ShaderSourceImportSettings::ShaderSourceImportSettings(const std::string& extension) : ImportSettings(".shader")
	{
	}
}
