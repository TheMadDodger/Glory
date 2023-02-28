#include "EditorShaderProcessor.h"
#include "ProjectSpace.h"
#include <Debug.h>
#include <fstream>
#include <ShaderManager.h>
#include <AssetManager.h>
#include <spirv_cross/spirv_glsl.hpp>
#include <FileLoaderModule.h>
#include <EditorAssetCallbacks.h>
#include <Engine.h>

namespace Glory::Editor
{
	std::map<UUID, EditorShaderData*> EditorShaderProcessor::m_pLoadedShaderData;
	std::queue<ShaderSourceData*> EditorShaderProcessor::m_ProcessShadersQueue;
	std::condition_variable EditorShaderProcessor::m_QueueCondition;
	std::mutex EditorShaderProcessor::m_QueueLock;
	ThreadedUMap<UUID, std::function<void(FileData*)>> EditorShaderProcessor::m_WaitingCallbacks;
	ThreadedVector<std::function<void(FileData*)>> EditorShaderProcessor::m_Callbacks;

	std::map<spirv_cross::SPIRType::BaseType, std::vector<uint32_t>> SpirBaseTypeToHash = {
		// Int
		{ spirv_cross::SPIRType::BaseType::Int,
		{ ResourceType::GetHash<int32_t>(), ResourceType::GetHash<glm::ivec2>(), ResourceType::GetHash<glm::ivec3>(), ResourceType::GetHash<glm::ivec4>() } },

		// UInt
		{ spirv_cross::SPIRType::BaseType::UInt,
		{ ResourceType::GetHash<uint32_t>(), ResourceType::GetHash<glm::uvec2>(), ResourceType::GetHash<glm::uvec3>(), ResourceType::GetHash<glm::uvec4>() } },

		// I64
		{ spirv_cross::SPIRType::BaseType::Int64,
		{ ResourceType::GetHash<int64_t>(), ResourceType::GetHash<glm::i64vec2>(), ResourceType::GetHash<glm::i64vec3>(), ResourceType::GetHash<glm::i64vec4>() } },

		// U64
		{ spirv_cross::SPIRType::BaseType::UInt64,
		{ ResourceType::GetHash<uint64_t>(), ResourceType::GetHash<glm::u64vec2>(), ResourceType::GetHash<glm::u64vec3>(), ResourceType::GetHash<glm::u64vec4>() } },

		// Float
		{ spirv_cross::SPIRType::BaseType::Float,
		{ ResourceType::GetHash<float>(), ResourceType::GetHash<glm::vec2>(), ResourceType::GetHash<glm::vec3>(), ResourceType::GetHash<glm::vec4>() } },

		// Bool
		{ spirv_cross::SPIRType::BaseType::Boolean,
		{ ResourceType::GetHash<bool>(), ResourceType::GetHash<glm::bvec2>(), ResourceType::GetHash<glm::bvec3>(), ResourceType::GetHash<glm::bvec4>() } },

		// Double
		{ spirv_cross::SPIRType::BaseType::Double, { ResourceType::GetHash<double>() } },

		// Short
		{ spirv_cross::SPIRType::BaseType::Short, { ResourceType::GetHash<short>() } },

		// UShort
		{ spirv_cross::SPIRType::BaseType::UShort, { ResourceType::GetHash<unsigned short>() } },

		// Unknown
		{ spirv_cross::SPIRType::BaseType::Unknown, { 0 } },
	};

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
			std::string cachePath = pProject->CachePath() + "\\CompiledShaders\\";
			return cachePath;
		});

		EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered, AssetRegisteredCallback);

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

			if (pShaderData == nullptr) continue;

			ProcessReflection(pShaderData);

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

		fileStream.seekg(0, std::ios::end);
		size_t size = (size_t)fileStream.tellg();
		fileStream.seekg(0, std::ios::beg);

		EditorShaderData* pShaderData = new EditorShaderData(pShaderSource->GetUUID());
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

	void EditorShaderProcessor::ProcessReflection(EditorShaderData* pEditorShader)
	{
		spirv_cross::Compiler compiler(pEditorShader->Data(), pEditorShader->Size());
		compiler.compile();
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		for (size_t i = 0; i < resources.sampled_images.size(); i++)
		{
			spirv_cross::Resource sampler = resources.sampled_images[i];
			pEditorShader->m_SamplerNames.push_back(sampler.name);
		}

		for (size_t i = 0; i < resources.storage_buffers.size(); i++)
		{
			spirv_cross::Resource storageBuffer = resources.storage_buffers[i];
			if (storageBuffer.name != "PropertiesSSBO") continue;
			const spirv_cross::SPIRType& base_type = compiler.get_type(storageBuffer.base_type_id);
			const spirv_cross::SPIRType& type = compiler.get_type(storageBuffer.type_id);

			if (base_type.basetype != spirv_cross::SPIRType::Struct) break;
			for (size_t j = 0; j < base_type.member_types.size(); j++)
			{
				spirv_cross::TypeID memberType = base_type.member_types[j];
				const spirv_cross::SPIRType& type = compiler.get_type(memberType);
				const std::string& name = compiler.get_member_name(storageBuffer.base_type_id, j);
				uint32_t hash = type.vecsize - 1 < SpirBaseTypeToHash[type.basetype].size() ? SpirBaseTypeToHash[type.basetype][type.vecsize - 1] : 0;
				pEditorShader->m_PropertyInfos.push_back(EditorShaderData::PropertyInfo(name, hash));
			}
		}
	}

	void EditorShaderProcessor::AssetRegisteredCallback(UUID uuid, const ResourceMeta& meta, Resource* pResource)
	{
		uint32_t typeHash = meta.Hash();
		size_t shaderSourceDataHash = ResourceType::GetHash<ShaderSourceData>();
		if (typeHash != shaderSourceDataHash) return;
		AssetManager::GetAsset(uuid, [](Resource* pLoadedResource)
		{
			if (!pLoadedResource) return;
			GetShaderSource((ShaderSourceData*)pLoadedResource);
		});
	}
}
