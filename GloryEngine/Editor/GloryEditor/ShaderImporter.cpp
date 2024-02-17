#include "ShaderImporter.h"
#include "EditorApplication.h"

#include <fstream>
#include <functional>
#include <sstream>
#include <map>
#include <Debug.h>

namespace Glory::Editor
{
    constexpr size_t NumSupportedExtensions = 9;
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
    };

    bool GetShaderTypeFromString(TemporaryShaderData& shaderData, const std::string& typeString)
    {
        auto itor = ShaderTypes.find(typeString);
        if (itor == ShaderTypes.end()) return false;
        shaderData.Type = itor->second;
        return true;
    }

    std::map<std::string, std::function<bool(TemporaryShaderData&, const std::string&)>> SymbolCallbacks = {
        { "type", GetShaderTypeFromString }
    };

    std::string_view ShaderImporter::Name() const
    {
        return "Internal Shader Importer";
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

    bool ProcessSymbol(TemporaryShaderData& shaderData, const std::string& symbol, const std::string& argument)
    {
        if (SymbolCallbacks.find(symbol) == SymbolCallbacks.end()) return false;
        return SymbolCallbacks[symbol](shaderData, argument);
    }

    void ProcessLine(TemporaryShaderData& shaderData, const std::string& line)
    {
        if (line[0] != '#')
        {
            AppendLine(line, shaderData.ProcessedSource);
            return;
        }

        size_t spaceIndex = line.find(' ');
        if (spaceIndex == std::string::npos) spaceIndex = line.length() - 1;
        std::string symbol = line.substr(1, spaceIndex - 1);
        if (symbol[symbol.length() - 1] == '\r') symbol = symbol.substr(0, symbol.length() - 1);

        std::string argument = line.substr(spaceIndex + 1);
        if (argument[argument.length() - 1] == '\r') argument = argument.substr(0, argument.length() - 1);

        if (!ProcessSymbol(shaderData, symbol, argument)) AppendLine(line, shaderData.ProcessedSource);
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
            ProcessLine(shaderData, line);
        }

        return { new ShaderSourceData(shaderData.Type, std::move(shaderData.Source), std::move(shaderData.ProcessedSource)) };
    }
}