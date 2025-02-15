#include "ShaderImporter.h"
#include "EditorApplication.h"
#include "EditorPipelineManager.h"

#include <fstream>
#include <functional>
#include <sstream>
#include <map>
#include <Debug.h>

namespace Glory::Editor
{
    constexpr size_t NumSupportedExtensions = 3;
    constexpr std::string_view SupportedExtensions[NumSupportedExtensions] = {
        ".vert",
        ".frag",
        ".shader"
    };

    std::map<std::string, ShaderType> ShaderTypes = {
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

    struct TemporaryShaderData
    {
        ShaderType Type;
        std::vector<char> Source;
        std::vector<char> ProcessedSource;
        std::vector<std::string> Features;
    };

    std::vector<std::filesystem::path> ShaderIncludeDirs;

    void AppendLine(const std::string& line, std::vector<char>& buffer);
    bool ProcessSymbol(TemporaryShaderData& shaderData, const std::string& symbol, const std::string& argument, const std::filesystem::path& path);
    void ProcessLine(TemporaryShaderData& shaderData, const std::string& line, const std::filesystem::path& path);

    bool GetShaderTypeFromString(TemporaryShaderData& shaderData, const std::string& typeString, const std::filesystem::path& path)
    {
        auto itor = ShaderTypes.find(typeString);
        if (itor == ShaderTypes.end()) return false;
        shaderData.Type = itor->second;
        return true;
    }

    bool ProcessInclude(TemporaryShaderData& shaderData, const std::string& includePath, const std::filesystem::path& path)
    {
        if (includePath.front() != '\"' || includePath.back() != includePath[0])
        {
            /* @todo: Log an error */
            return false;
        }

        const std::string actualInclude = includePath.substr(1, includePath.size() - 2);

        std::filesystem::path pathToFile = ShaderImporter::FindShaderInclude(actualInclude);

        if (!std::filesystem::exists(pathToFile))
            pathToFile = path.parent_path().append(actualInclude);

        if (!std::filesystem::exists(pathToFile))
        {
            EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("Could not open shader include: " + includePath + " File not found.");
            return nullptr;
        }

        TemporaryShaderData includeShader;
        includeShader.Type = ShaderType::ST_Unknown;

        std::ifstream file(pathToFile, std::ios::binary);

        if (!file.is_open())
        {
            EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("Could not open file: " + pathToFile.string());
            return nullptr;
        }

        file.seekg(0, std::ios_base::end);
        size_t fileSize = (size_t)file.tellg();
        includeShader.Source.resize(fileSize);
        file.seekg(0, std::ios_base::beg);
        file.read(includeShader.Source.data(), fileSize);
        file.close();

        std::string originalSourceString(includeShader.Source.begin(), includeShader.Source.end());
        std::istringstream stream(originalSourceString);
        std::string line;

        for (std::string line; std::getline(stream, line); )
        {
            ProcessLine(includeShader, line, pathToFile);
        }

        const size_t currentSize = shaderData.ProcessedSource.size();
        shaderData.ProcessedSource.resize(currentSize + includeShader.ProcessedSource.size());
        std::memcpy(&shaderData.ProcessedSource[currentSize], includeShader.ProcessedSource.data(), includeShader.ProcessedSource.size());
        return true;
    }

    bool ProcessDefine(TemporaryShaderData& shaderData, const std::string& define, const std::filesystem::path& path)
    {
        if (define._Starts_with("FEATURE_"))
        {
            shaderData.Features.push_back(define);
            return true;
        }
        return false;
    }

    std::map<std::string, std::function<bool(TemporaryShaderData&, const std::string&, const std::filesystem::path&)>> SymbolCallbacks = {
        { "type", GetShaderTypeFromString },
        { "include", ProcessInclude },
        { "define", ProcessDefine }
    };

    std::string_view ShaderImporter::Name() const
    {
        return "Internal Shader Importer";
    }

    std::filesystem::path ShaderImporter::FindShaderInclude(const std::filesystem::path& path)
    {
        for (const std::filesystem::path& includeDir : ShaderIncludeDirs)
        {
            std::filesystem::path searchPath = includeDir;
            searchPath.append(path.string());
            if (!std::filesystem::exists(searchPath)) continue;
            return searchPath;
        }
        return path;
    }

    bool ShaderImporter::SupportsExtension(const std::filesystem::path& extension) const
    {
        for (size_t i = 0; i < NumSupportedExtensions; ++i)
        {
            if (extension.compare(SupportedExtensions[i]) != 0) continue;
            return true;
        }
        return false;
    }

    void AppendLine(const std::string& line, std::vector<char>& buffer)
    {
        for (size_t i = 0; i < line.length(); i++) buffer.push_back(line[i]);
        buffer.push_back('\n');
    }

    bool ProcessSymbol(TemporaryShaderData& shaderData, const std::string& symbol, const std::string& argument, const std::filesystem::path& path)
    {
        if (SymbolCallbacks.find(symbol) == SymbolCallbacks.end()) return false;
        return SymbolCallbacks[symbol](shaderData, argument, path);
    }

    void ProcessLine(TemporaryShaderData& shaderData, const std::string& line, const std::filesystem::path& path)
    {
        if (line[0] != '#')
        {
            AppendLine(line, shaderData.ProcessedSource);
            return;
        }

        size_t spaceIndex = line.find(' ');
        if (spaceIndex == std::string::npos) spaceIndex = line.length() - 1;
        if (line[spaceIndex] == '\r')
        {
            AppendLine(line, shaderData.ProcessedSource);
            return;
        }

        std::string symbol = line.substr(1, spaceIndex - 1);
        if (symbol[symbol.length() - 1] == '\r') symbol = symbol.substr(0, symbol.length() - 1);

        std::string argument = line.substr(spaceIndex + 1);
        if (argument[argument.length() - 1] == '\r') argument = argument.substr(0, argument.length() - 1);

        if (!ProcessSymbol(shaderData, symbol, argument, path)) AppendLine(line, shaderData.ProcessedSource);
    }

    ImportedResource ShaderImporter::LoadResource(const std::filesystem::path& path, void*) const
    {
        std::ifstream file(path, std::ios::binary);

        if (!file.is_open())
        {
            EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("Could not open file: " + path.string());
            return nullptr;
        }

        TemporaryShaderData shaderData;

        file.seekg(0, std::ios_base::end);
        size_t fileSize = (size_t)file.tellg();
        shaderData.Source.resize(fileSize);
        file.seekg(0, std::ios_base::beg);
        file.read(shaderData.Source.data(), fileSize);
        file.close();

        /* @fixme This is 2 new string allocations */
        std::string originalSourceString(shaderData.Source.begin(), shaderData.Source.end());
        std::istringstream stream(originalSourceString);
        std::string line;

        for (std::string line; std::getline(stream, line); )
        {
            ProcessLine(shaderData, line, path);
        }

        return { path, new ShaderSourceData(shaderData.Type, std::move(shaderData.Source),
            std::move(shaderData.ProcessedSource), std::move(shaderData.Features)) };
    }

    void ShaderImporter::Initialize()
    {
        Engine* pEngine = EditorApplication::GetInstance()->GetEngine();

        for (size_t i = 0; i < pEngine->ModulesCount(); ++i)
        {
            Module* pModule = pEngine->GetModule(i);
            std::filesystem::path path = pModule->GetPath();
            path.append("Assets/Shaders");
            if (!std::filesystem::exists(path)) continue;
            ShaderIncludeDirs.push_back(path);
        }
    }
}