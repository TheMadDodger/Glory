#include "EditorShaderProcessor.h"
#include "ProjectSpace.h"
#include <Debug.h>
#include <fstream>
#include <ShaderManager.h>
#include <AssetManager.h>
#include <spirv_glsl.hpp>
#include <FileLoaderModule.h>
#include <AssetCallbacks.h>

namespace Glory::Editor
{
	std::map<UUID, EditorShaderData*> EditorShaderProcessor::m_pLoadedShaderData;
	std::queue<ShaderSourceData*> EditorShaderProcessor::m_ProcessShadersQueue;
	std::condition_variable EditorShaderProcessor::m_QueueCondition;
	std::mutex EditorShaderProcessor::m_QueueLock;
	ThreadedUMap<UUID, std::function<void(FileData*)>> EditorShaderProcessor::m_WaitingCallbacks;
	ThreadedVector<std::function<void(FileData*)>> EditorShaderProcessor::m_Callbacks;

	EditorShaderData* EditorShaderProcessor::GetShaderSource(ShaderSourceData* pShaderSource)
	{
		UUID uuid = pShaderSource->GetUUID();
		std::unique_lock<std::mutex> lock(m_QueueLock);

		if (m_pLoadedShaderData.find(uuid) != m_pLoadedShaderData.end())
		{
			lock.unlock();
			return m_pLoadedShaderData[uuid];
		}

		m_ProcessShadersQueue.push(pShaderSource);
		m_QueueCondition.notify_one();
		lock.unlock();
		return nullptr;
	}

	EditorShaderProcessor::EditorShaderProcessor() : m_pThread(nullptr)
	{
		m_ShaderTypeToKind = {
			{ ShaderType::ST_Compute, shaderc_shader_kind::shaderc_compute_shader },
			{ ShaderType::ST_Fragment, shaderc_shader_kind::shaderc_fragment_shader },
			{ ShaderType::ST_Geomtery, shaderc_shader_kind::shaderc_geometry_shader },
			{ ShaderType::ST_TessControl, shaderc_shader_kind::shaderc_tess_control_shader },
			{ ShaderType::ST_TessEval, shaderc_shader_kind::shaderc_tess_evaluation_shader },
			{ ShaderType::ST_Vertex, shaderc_shader_kind::shaderc_vertex_shader },
		};
	}

	EditorShaderProcessor::~EditorShaderProcessor()
	{
	}

	void EditorShaderProcessor::Start()
	{
		ShaderManager::OverrideCompiledShadersPathFunc([]()
		{
			ProjectSpace* pProject = ProjectSpace::GetOpenProject();
			if (pProject == nullptr) return std::string("");
			std::string cachePath = pProject->CachePath() + "CompiledShaders\\";
			return cachePath;
		});

		AssetCallbacks::RegisterCallback(CallbackType::CT_AssetRegistered, AssetRegisteredCallback);

		//ShaderManager::OverrideMissingShaderHandlerFunc([&](UUID uuid, std::function<void(FileData*)> callback)
		//{
		//	ShaderSourceData* pShaderSource = AssetManager::GetAssetImmediate<ShaderSourceData>(uuid);
		//	EditorShaderData* pEditorShader = GetShaderSource(pShaderSource);
		//	if (pEditorShader)
		//	{
		//		CompileForCurrentPlatform(pEditorShader);
		//	}
		//	m_WaitingCallbacks.Set(uuid, callback);
		//});

		m_pThread = ThreadManager::Run(std::bind(&EditorShaderProcessor::ThreadLoop, this));
	}

	void EditorShaderProcessor::Stop()
	{
		std::unique_lock<std::mutex> lock(m_QueueLock);
		m_Exit = true;
		m_QueueCondition.notify_one();
		lock.unlock();
	}

	void EditorShaderProcessor::ThreadLoop()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(m_QueueLock);
			m_QueueCondition.wait(lock, [&] { return m_ProcessShadersQueue.size() > 0 || m_Exit; });
			if (m_Exit)
			{
				lock.unlock();
				break;
			}

			ShaderSourceData* pShaderSource = m_ProcessShadersQueue.front();
			UUID uuid = pShaderSource->GetUUID();
			m_ProcessShadersQueue.pop();
			lock.unlock();

			ProjectSpace* pProject = ProjectSpace::GetOpenProject();
			std::filesystem::path cacheRoot = pProject->CachePath();
			std::filesystem::path shaderSourceCache = cacheRoot.append("ShaderSource");
			std::filesystem::path cachedShaderSourceFile = shaderSourceCache.append(std::to_string(uuid));

			EditorShaderData* pShaderData = std::filesystem::exists(cachedShaderSourceFile) ? LoadCache(pShaderSource, cachedShaderSourceFile)
				: CompileAndCache(pShaderSource, cachedShaderSourceFile);

			std::string path = ShaderManager::GetCompiledShaderPath(pShaderData->GetUUID());
			CompileForCurrentPlatform(pShaderData, path);

			FileImportSettings importSettings;
			importSettings.AddNullTerminateAtEnd = true;
			importSettings.Flags = std::ios::ate | std::ios::binary;
			importSettings.m_Extension = "";
			FileData* pCompiledShader = (FileData*)Game::GetGame().GetEngine()->GetLoaderModule<FileData>()->Load(path, importSettings);
			pShaderSource->SetCompiledShader(pCompiledShader);
		}
	}

	EditorShaderData* EditorShaderProcessor::CompileAndCache(ShaderSourceData* pShaderSource, std::filesystem::path path)
	{
		ShaderType shaderType = pShaderSource->GetShaderType();
		if (m_ShaderTypeToKind.find(shaderType) == m_ShaderTypeToKind.end())
		{
			Debug::LogError("Shader " + pShaderSource->Name() + " compilation failed due to unknown shader type.");
			return nullptr;
		}

		shaderc::Compiler compiler;
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(pShaderSource->Data(), pShaderSource->Size(), m_ShaderTypeToKind[shaderType], pShaderSource->Name().data());

		size_t errors = result.GetNumErrors();
		size_t warnings = result.GetNumWarnings();

		if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
		{
			Debug::LogError("Shader " + pShaderSource->Name() + " compilation failed with " + std::to_string(errors) + " errors and " + std::to_string(warnings) + " warnings.");
			Debug::LogError(result.GetErrorMessage());
			return nullptr;
		}
		else
			Debug::LogInfo("Shader " + pShaderSource->Name() + " compilation succeeded with " + std::to_string(errors) + " errors and " + std::to_string(warnings) + " warnings.");

		if (warnings > 0)
			Debug::LogWarning(result.GetErrorMessage());

		EditorShaderData* pShaderData = new EditorShaderData(pShaderSource->GetUUID());
		pShaderData->m_ShaderData.assign(result.begin(), result.end());

		std::unique_lock<std::mutex> lock(m_QueueLock);
		m_pLoadedShaderData[pShaderSource->GetUUID()] = pShaderData;
		lock.unlock();

		std::ofstream fileStream(path, std::ios::binary);
		size_t size = pShaderData->m_ShaderData.size() * sizeof(uint32_t);
		fileStream.write((const char*)pShaderData->m_ShaderData.data(), size);
		fileStream.close();
		return pShaderData;
	}

	EditorShaderData* EditorShaderProcessor::LoadCache(ShaderSourceData* pShaderSource, std::filesystem::path path)
	{
		std::ifstream fileStream(path, std::ios::binary);
		if (!fileStream.is_open()) return nullptr;

		EditorShaderData* pShaderData = new EditorShaderData(pShaderSource->GetUUID());
		fileStream.seekg(0, std::ios::end);
		size_t size = (size_t)fileStream.tellg();
		fileStream.seekg(0, std::ios::beg);

		pShaderData->m_ShaderData.resize(size / sizeof(uint32_t));
		fileStream.read((char*)pShaderData->m_ShaderData.data(), size);
		fileStream.close();

		std::unique_lock<std::mutex> lock(m_QueueLock);
		m_pLoadedShaderData[pShaderSource->GetUUID()] = pShaderData;
		lock.unlock();
		return pShaderData;
	}

	void EditorShaderProcessor::CompileForCurrentPlatform(EditorShaderData* pEditorShader, const std::string& path)
	{
		spirv_cross::CompilerGLSL compiler(pEditorShader->Data(), pEditorShader->Size());
		std::string source = compiler.compile();
		std::ofstream stream(path);
		stream.write(source.data(), source.size());
		stream.close();
	}

	void EditorShaderProcessor::AssetRegisteredCallback(UUID uuid, const ResourceMeta& meta, Resource* pResource)
	{
		size_t typeHash = meta.Hash();
		size_t shaderSourceDataHash = ResourceType::GetHash<ShaderSourceData>();
		if (typeHash != shaderSourceDataHash) return;
		AssetManager::GetAsset(uuid, [](Resource* pLoadedResource)
		{
			if (!pLoadedResource) return;
			GetShaderSource((ShaderSourceData*)pLoadedResource);
		});
	}
}
