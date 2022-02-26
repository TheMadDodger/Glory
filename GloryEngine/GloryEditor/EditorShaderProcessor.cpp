#include "EditorShaderProcessor.h"
#include "ProjectSpace.h"
#include <Debug.h>
#include <fstream>

namespace Glory::Editor
{
	std::map<UUID, EditorShaderData*> EditorShaderProcessor::m_pLoadedShaderData;
	std::queue<ShaderSourceData*> EditorShaderProcessor::m_ProcessShadersQueue;
	std::condition_variable EditorShaderProcessor::m_QueueCondition;
	std::mutex EditorShaderProcessor::m_QueueLock;

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
			pProject->CreateFolder("Cache/ShaderSource");
			std::filesystem::path shaderSourceCache = cacheRoot.append("ShaderSource");
			std::filesystem::path cachedShaderSourceFile = shaderSourceCache.append(std::to_string(uuid));

			if (std::filesystem::exists(cachedShaderSourceFile))
			{
				LoadCache(pShaderSource, cachedShaderSourceFile);
				continue;
			}

			CompileAndCache(pShaderSource, cachedShaderSourceFile);
		}
	}

	void EditorShaderProcessor::CompileAndCache(ShaderSourceData* pShaderSource, std::filesystem::path path)
	{
		ShaderType shaderType = pShaderSource->GetShaderType();
		if (m_ShaderTypeToKind.find(shaderType) == m_ShaderTypeToKind.end())
		{
			Debug::LogError("Shader " + pShaderSource->Name() + " compilation failed due to unknown shader type.");
			return;
		}

		shaderc::Compiler compiler;
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(pShaderSource->Data(), pShaderSource->Size(), m_ShaderTypeToKind[shaderType], pShaderSource->Name().data());

		size_t errors = result.GetNumErrors();
		size_t warnings = result.GetNumWarnings();

		if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
		{
			Debug::LogError("Shader " + pShaderSource->Name() + " compilation failed with " + std::to_string(errors) + " errors and " + std::to_string(warnings) + " warnings.");
			Debug::LogError(result.GetErrorMessage());
			return;
		}
		else
			Debug::LogInfo("Shader " + pShaderSource->Name() + " compilation succeeded with " + std::to_string(errors) + " errors and " + std::to_string(warnings) + " warnings.");

		if (warnings > 0)
			Debug::LogWarning(result.GetErrorMessage());

		EditorShaderData* pShaderData = new EditorShaderData();
		pShaderData->m_ShaderData.assign(result.begin(), result.end());

		std::unique_lock<std::mutex> lock(m_QueueLock);
		m_pLoadedShaderData[pShaderSource->GetUUID()] = pShaderData;
		lock.unlock();

		std::ofstream fileStream(path, std::ios::binary);
		size_t size = pShaderData->m_ShaderData.size() * sizeof(uint32_t);
		fileStream.write((const char*)pShaderData->m_ShaderData.data(), size);
		fileStream.close();
	}

	void EditorShaderProcessor::LoadCache(ShaderSourceData* pShaderSource, std::filesystem::path path)
	{
		std::ifstream fileStream(path, std::ios::binary);
		if (!fileStream.is_open()) return;

		EditorShaderData* pShaderData = new EditorShaderData();
		fileStream.seekg(0, std::ios::end);
		size_t size = (size_t)fileStream.tellg();
		fileStream.seekg(0, std::ios::beg);

		pShaderData->m_ShaderData.resize(size / sizeof(uint32_t));
		fileStream.read((char*)pShaderData->m_ShaderData.data(), size);
		fileStream.close();


		std::unique_lock<std::mutex> lock(m_QueueLock);
		m_pLoadedShaderData[pShaderSource->GetUUID()] = pShaderData;
		lock.unlock();
	}
}
