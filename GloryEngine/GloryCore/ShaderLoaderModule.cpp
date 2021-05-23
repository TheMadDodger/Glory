#include "ShaderLoaderModule.h"
#include "Game.h"
#include "FileLoaderModule.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace Glory
{
	ShaderLoaderModule::ShaderLoaderModule()
	{
	}

	ShaderLoaderModule::~ShaderLoaderModule()
	{
	}

	const std::type_info& ShaderLoaderModule::GetModuleType()
	{
		return typeid(ShaderLoaderModule);
	}

	std::vector<uint32_t>& ShaderLoaderModule::GetData(ShaderData* pShaderData)
	{
		return pShaderData->m_ShaderData;
	}

	ShaderData* ShaderLoaderModule::LoadResource(const std::string& path, const ShaderImportSettings& importSettings)
	{
		return LoadShader(path, importSettings);

		//ShaderData* pShaderData = new ShaderData();
		//pShaderData->m_ProcessedSource = PreProcessFile(pShaderData, path);
		//std::cout << pShaderData->m_ProcessedSource << std::endl;
		//return pShaderData;

		//ShaderData* pShaderData = LoadShader(path, importSettings);
		//pTexture->BuildTexture();
		//return pShaderData;
	}

	//void ShaderLoaderModule::Initialize()
	//{
	//	m_DirectiveCallbacks["include"] = [&](ShaderData* pShaderData, const std::string& path, int lineIndex, std::string& line, const std::string& directive, const std::string& args)
	//	{ return IncludeDirectiveCallback(pShaderData, path, lineIndex, line, directive, args); };
	//
	//	m_DirectiveCallbacks["version"] = [&](ShaderData* pShaderData, const std::string& path, int lineIndex, std::string& line, const std::string& directive, const std::string& args)
	//	{ return VersionCallback(pShaderData, path, lineIndex, line, directive, args); };
	//}
	//
	//bool ShaderLoaderModule::IncludeDirectiveCallback(ShaderData* pShaderData, const std::string& path, int lineIndex, std::string& line, const std::string& directive, const std::string& args)
	//{
	//	if (args.length() <= 2)
	//	{
	//		//Debug::LogError("ShaderLoaderModule::ProcessDirective > Syntax error: " + line);
	//		line = "";
	//		return false;
	//	}
	//	std::string relativePath = args.substr(1, args.length() - 2);
	//
	//	std::filesystem::path filePath = path;
	//	std::filesystem::path folderPath = filePath.parent_path();
	//	folderPath.append(relativePath);
	//
	//	pShaderData->m_IncludedFiles.push_back(folderPath.string());
	//
	//	line = PreProcessFile(pShaderData, folderPath.string());
	//	return true;
	//}
	//
	//bool ShaderLoaderModule::VersionCallback(ShaderData* pShaderData, const std::string& path, int lineIndex, std::string& line, const std::string& directive, const std::string& args)
	//{
	//	pShaderData->m_Version = (size_t)std::stoi(args);
	//	return true;
	//}
	//
	//std::string ShaderLoaderModule::PreProcessFile(ShaderData* pShaderData, const std::string& path)
	//{
	//	std::string source = "";
	//	std::ifstream file(path);
	//
	//	if (!file.is_open())
	//	{
	//		Debug::LogError("Could not open file: " + path);
	//		return "";
	//	}
	//
	//	std::string line;
	//
	//	int lineIndex = 0;
	//	while (!file.eof())
	//	{
	//		std::getline(file, line);
	//		PreProcessLine(pShaderData, path, lineIndex, line);
	//		source += line + '\n';
	//		++lineIndex;
	//	}
	//	return source;
	//}
	//
	//void ShaderLoaderModule::PreProcessLine(ShaderData* pShaderData, const std::string& path, int lineIndex, std::string& line)
	//{
	//	if (line.length() <= 0) return;
	//	if (line[0] == DIRECTIVECHAR)
	//		return ProcessDirectiveLine(pShaderData, path, lineIndex, line);
	//	else if (line[0] == PROPERTYSTARTCHAR)
	//		return ProcessPropertyLine(pShaderData, path, lineIndex, line);
	//}
	//
	//void ShaderLoaderModule::ProcessDirectiveLine(ShaderData* pShaderData, const std::string& path, int lineIndex, std::string& line)
	//{
	//	if (line.length() <= 1)
	//	{
	//		Debug::LogError("ShaderLoaderModule::ProcessDirective > Syntax error: " + line);
	//		line = "";
	//		return;
	//	}
	//
	//	int spaceIndex = line.find(' ');
	//	std::string directive = line.substr(1, spaceIndex - 1);
	//	std::string data = line.substr(spaceIndex + 1);
	//	auto it = m_DirectiveCallbacks.find(directive);
	//	if (it == m_DirectiveCallbacks.end()) return;
	//	auto callback = (*it).second;
	//
	//	if (!callback(pShaderData, path, lineIndex, line, directive, data))
	//	{
	//		Debug::LogError("ShaderLoaderModule::ProcessDirective > Syntax error: " + line);
	//		line = "";
	//		return;
	//	}
	//}
	//
	//void ShaderLoaderModule::ProcessPropertyLine(ShaderData* pShaderData, const std::string& path, int lineIndex, std::string& line)
	//{
	//
	//
	//	// Property lines cant be compiled so we remove it when we are done!
	//	line = "";
	//}
}
