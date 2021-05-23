#include "GLSLShaderLoader.h"
#include <Debug.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace Glory
{
	const std::string GLSLShaderLoader::SHADERCACHEPATH = "./Assets/cache/";

	GLSLShaderLoader::GLSLShaderLoader()
	{
	}

	GLSLShaderLoader::~GLSLShaderLoader()
	{
	}

	ShaderData* GLSLShaderLoader::LoadShader(const std::string& path, const ShaderImportSettings& importSettings)
	{
		ShaderData* pShaderData = new ShaderData();
		std::vector<uint32_t>& shaderData = GetData(pShaderData);
		GetOrCreateSpirVShaderCache(path, importSettings, shaderData);
		return pShaderData;
	}

	void GLSLShaderLoader::GetOrCreateSpirVShaderCache(const std::string& path, const ShaderImportSettings& importSettings, std::vector<uint32_t>& data)
	{
		std::string source = LoadFile(path);
		std::string cachePath = GetCachePath(source);
		std::ifstream file(cachePath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			return CreateSpirVShaderCache(path, cachePath, source, importSettings, data);

		LoadCache(file, data);
	}

	void GLSLShaderLoader::CreateSpirVShaderCache(const std::string& path, const std::string& cachePath, const std::string& source, const ShaderImportSettings& importSettings, std::vector<uint32_t>& data)
	{
		// Create the cache
		shaderc::Compiler compiler;
		shaderc::CompileOptions compilerOptions;
		compilerOptions.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
		for (size_t i = 0; i < importSettings.CompilerDefinitions.size(); i++)
		{
			std::string definition = importSettings.CompilerDefinitions[i];
			compilerOptions.AddMacroDefinition(definition);
		}

		shaderc_shader_kind shaderSourceKind = GetShaderSourceKind(path);

		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, shaderSourceKind, path.c_str(), compilerOptions);
		if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
		{
			Debug::LogError("Error compiling shader at" + path + " - Message: " + result.GetErrorMessage());
			return;
		}

		data = std::vector<uint32_t>(result.cbegin(), result.cend());

		std::ofstream outFile(cachePath, std::ios::out | std::ios::binary);
		if (!outFile.is_open())
		{
			Debug::LogError("Could not write cache file for shader: " + path);
			return;
		}

		outFile.write((char*)data.data(), data.size() * sizeof(uint32_t));
		outFile.flush();
		outFile.close();
	}

	std::string GLSLShaderLoader::LoadFile(const std::string& path)
	{
		std::ifstream file(path);

		if (!file.is_open())
		{
			Debug::LogError("Could not open file: " + path);
			return "";
		}

		std::vector<char> buffer;
		file.seekg(0, std::ios::end);
		size_t fileSize = (size_t)file.tellg();
		buffer.resize(fileSize);
		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), fileSize);
		file.close();

		std::string source = buffer.data();
		return source;
	}

	void GLSLShaderLoader::LoadCache(std::ifstream& file, std::vector<uint32_t>& data)
	{
		file.seekg(0, std::ios::end);
		size_t size = (size_t)file.tellg();
		file.seekg(0, std::ios::beg);

		data.resize(size / sizeof(uint32_t));
		file.read((char*)data.data(), size);
		file.close();
	}

	std::string GLSLShaderLoader::GetCachePath(const std::string& source)
	{
		std::hash<std::string> hasher;
		size_t hash = hasher(source);
		std::stringstream stream;
		stream << SHADERCACHEPATH << hash << ".spv";
		return stream.str();
	}

	shaderc_shader_kind GLSLShaderLoader::GetShaderSourceKind(const std::string& path)
	{
		std::filesystem::path filePath = path;
		std::string extension = filePath.extension().string();

		if (extension == ".vert") return shaderc_shader_kind::shaderc_vertex_shader;
		if (extension == ".frag") return shaderc_shader_kind::shaderc_fragment_shader;
		if (extension == ".comp") return shaderc_shader_kind::shaderc_compute_shader;
		if (extension == ".geom") return shaderc_shader_kind::shaderc_geometry_shader;
		if (extension == ".te") return shaderc_shader_kind::shaderc_tess_control_shader;
		if (extension == ".tc") return shaderc_shader_kind::shaderc_tess_evaluation_shader;

		return shaderc_shader_kind();
	}
}
