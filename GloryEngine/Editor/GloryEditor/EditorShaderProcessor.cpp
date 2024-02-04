#include "EditorShaderProcessor.h"
#include "ProjectSpace.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "Importer.h"
#include "EditorShaderData.h"
#include "Dispatcher.h"

#include <ShaderSourceData.h>
#include <Debug.h>
#include <fstream>
#include <ShaderManager.h>
#include <AssetManager.h>
#include <spirv_cross/spirv_glsl.hpp>
#include <FileLoaderModule.h>
#include <EditorAssetCallbacks.h>
#include <Engine.h>
#include <EditorAssetDatabase.h>

namespace Glory::Editor
{
	ThreadedUMap<UUID, ShaderSourceData*> EditorShaderProcessor::m_pLoadedShaderSources;
	ThreadedUMap<UUID, EditorShaderData*> EditorShaderProcessor::m_pCompiledShaders;
	ThreadedVector<UUID> EditorShaderProcessor::m_QueuedShaders;
	ThreadedVector<UUID> EditorShaderProcessor::m_FinishedShaders;

	std::map<ShaderType, shaderc_shader_kind> ShaderTypeToKind = {
		{ ShaderType::ST_Compute, shaderc_shader_kind::shaderc_compute_shader },
		{ ShaderType::ST_Fragment, shaderc_shader_kind::shaderc_fragment_shader },
		{ ShaderType::ST_Geomtery, shaderc_shader_kind::shaderc_geometry_shader },
		{ ShaderType::ST_TessControl, shaderc_shader_kind::shaderc_tess_control_shader },
		{ ShaderType::ST_TessEval, shaderc_shader_kind::shaderc_tess_evaluation_shader },
		{ ShaderType::ST_Vertex, shaderc_shader_kind::shaderc_vertex_shader },
	};

	Jobs::JobPool<bool, UUID>* EditorShaderProcessor::m_pShaderJobsPool = nullptr;

	std::map<spirv_cross::SPIRType::BaseType, std::vector<uint32_t>> SpirBaseTypeToHash = {
		// Int
		{ spirv_cross::SPIRType::BaseType::Int,
		{ ResourceTypes::GetHash<int32_t>(), ResourceTypes::GetHash<glm::ivec2>(), ResourceTypes::GetHash<glm::ivec3>(), ResourceTypes::GetHash<glm::ivec4>() } },

		// UInt
		{ spirv_cross::SPIRType::BaseType::UInt,
		{ ResourceTypes::GetHash<uint32_t>(), ResourceTypes::GetHash<glm::uvec2>(), ResourceTypes::GetHash<glm::uvec3>(), ResourceTypes::GetHash<glm::uvec4>() } },

		// I64
		{ spirv_cross::SPIRType::BaseType::Int64,
		{ ResourceTypes::GetHash<int64_t>(), ResourceTypes::GetHash<glm::i64vec2>(), ResourceTypes::GetHash<glm::i64vec3>(), ResourceTypes::GetHash<glm::i64vec4>() } },

		// U64
		{ spirv_cross::SPIRType::BaseType::UInt64,
		{ ResourceTypes::GetHash<uint64_t>(), ResourceTypes::GetHash<glm::u64vec2>(), ResourceTypes::GetHash<glm::u64vec3>(), ResourceTypes::GetHash<glm::u64vec4>() } },

		// Float
		{ spirv_cross::SPIRType::BaseType::Float,
		{ ResourceTypes::GetHash<float>(), ResourceTypes::GetHash<glm::vec2>(), ResourceTypes::GetHash<glm::vec3>(), ResourceTypes::GetHash<glm::vec4>() } },

		// Bool
		{ spirv_cross::SPIRType::BaseType::Boolean,
		{ ResourceTypes::GetHash<bool>(), ResourceTypes::GetHash<glm::bvec2>(), ResourceTypes::GetHash<glm::bvec3>(), ResourceTypes::GetHash<glm::bvec4>() } },

		// Double
		{ spirv_cross::SPIRType::BaseType::Double, { ResourceTypes::GetHash<double>() } },

		// Short
		{ spirv_cross::SPIRType::BaseType::Short, { ResourceTypes::GetHash<short>() } },

		// UShort
		{ spirv_cross::SPIRType::BaseType::UShort, { ResourceTypes::GetHash<unsigned short>() } },

		// Unknown
		{ spirv_cross::SPIRType::BaseType::Unknown, { 0 } },
	};

	ShaderSourceData* EditorShaderProcessor::GetShaderSource(UUID uuid)
	{
		if (m_pLoadedShaderSources.Contains(uuid))
			return m_pLoadedShaderSources[uuid];
		return nullptr;
	}

	EditorShaderData* EditorShaderProcessor::GetEditorShader(UUID uuid)
	{
		if (m_pCompiledShaders.Contains(uuid))
			return m_pCompiledShaders[uuid];
		return nullptr;
	}

	EditorShaderProcessor::EditorShaderProcessor() : m_AssetRegisteredCallback(0), m_AssetUpdatedCallback(0)
	{
	}

	EditorShaderProcessor::~EditorShaderProcessor()
	{
	}

	void EditorShaderProcessor::Start()
	{
		EditorApplication::GetInstance()->GetEngine()->GetShaderManager().OverrideCompiledShadersPathFunc([]()
		{
			ProjectSpace* pProject = ProjectSpace::GetOpenProject();
			if (pProject == nullptr) return std::string("");
			std::string cachePath = pProject->CachePath() + "\\CompiledShaders\\";
			return cachePath;
		});

		m_AssetRegisteredCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered, AssetCallback);
		m_AssetUpdatedCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetUpdated, AssetUpdatedCallback);

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

		m_pShaderJobsPool = Jobs::JobManager::Run<bool, UUID>();
	}

	void EditorShaderProcessor::Stop()
	{
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetRegisteredCallback);
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetUpdatedCallback);
	}

	EditorShaderData* EditorShaderProcessor::CompileAndCache(ShaderSourceData* pShaderSource, std::filesystem::path path)
	{
		ShaderType shaderType = pShaderSource->GetShaderType();
		if (ShaderTypeToKind.find(shaderType) == ShaderTypeToKind.end())
		{
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("Shader " + pShaderSource->Name() + " compilation failed due to unknown shader type.");
			return nullptr;
		}

		shaderc::Compiler compiler;
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(pShaderSource->Data(), pShaderSource->Size(), ShaderTypeToKind.at(shaderType), pShaderSource->Name().data());

		size_t errors = result.GetNumErrors();
		size_t warnings = result.GetNumWarnings();

		if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
		{
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("Shader " + pShaderSource->Name() + " compilation failed with " + std::to_string(errors) + " errors and " + std::to_string(warnings) + " warnings.");
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError(result.GetErrorMessage());
			return nullptr;
		}
		else
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo("Shader " + pShaderSource->Name() + " compilation succeeded with " + std::to_string(errors) + " errors and " + std::to_string(warnings) + " warnings.");

		if (warnings > 0)
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogWarning(result.GetErrorMessage());

		EditorShaderData* pShaderData = new EditorShaderData(pShaderSource->GetUUID());
		pShaderData->m_ShaderData.assign(result.begin(), result.end());

		if (m_pCompiledShaders.Contains(pShaderSource->GetUUID()))
		{
			m_pCompiledShaders.Do(pShaderSource->GetUUID(), [&](EditorShaderData** val)
			{
				/* Replace */
				delete *val;
				*val = pShaderData;
			});
		}
		else
		{
			m_pCompiledShaders.Set(pShaderSource->GetUUID(), pShaderData);
		}

		std::ofstream fileStream(path, std::ios::binary);
		size_t size = pShaderData->m_ShaderData.size() * sizeof(uint32_t);
		fileStream.write((const char*)pShaderData->m_ShaderData.data(), size);
		fileStream.close();
		return pShaderData;
	}

	EditorShaderData* EditorShaderProcessor::LoadCache(UUID shaderID, std::filesystem::path path)
	{
		std::ifstream fileStream(path, std::ios::binary);
		if (!fileStream.is_open()) return nullptr;

		fileStream.seekg(0, std::ios::end);
		size_t size = (size_t)fileStream.tellg();
		fileStream.seekg(0, std::ios::beg);

		EditorShaderData* pShaderData = new EditorShaderData(shaderID);
		pShaderData->m_ShaderData.resize(size / sizeof(uint32_t));
		fileStream.read((char*)pShaderData->m_ShaderData.data(), size);
		fileStream.close();

		if (m_pCompiledShaders.Contains(shaderID))
		{
			m_pCompiledShaders.Do(shaderID, [&](EditorShaderData** val)
			{
				/* Replace */
				delete* val;
				*val = pShaderData;
			});
		}
		else
		{
			m_pCompiledShaders.Set(shaderID, pShaderData);
		}
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
			for (uint32_t j = 0; j < (uint32_t)base_type.member_types.size(); j++)
			{
				spirv_cross::TypeID memberType = base_type.member_types[j];
				const spirv_cross::SPIRType& type = compiler.get_type(memberType);
				const std::string& name = compiler.get_member_name(storageBuffer.base_type_id, j);
				uint32_t hash = type.vecsize - 1 < SpirBaseTypeToHash[type.basetype].size() ? SpirBaseTypeToHash[type.basetype][type.vecsize - 1] : 0;
				pEditorShader->m_PropertyInfos.push_back(EditorShaderData::PropertyInfo(name, hash));
			}
		}
	}

	void EditorShaderProcessor::RunCallbacks()
	{
		m_FinishedShaders.ForEachClear([](const UUID& uuid) {
			ShaderCompiledEventDispatcher().Enqueue({ uuid });
		});
		ShaderCompiledEventDispatcher().Flush();
	}

	EditorShaderProcessor::ShaderCompiledDispatcher& EditorShaderProcessor::ShaderCompiledEventDispatcher()
	{
		static ShaderCompiledDispatcher dispatcher;
		return dispatcher;
	}

	void EditorShaderProcessor::AssetCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta))
			return;
		const uint32_t typeHash = meta.Hash();
		static const size_t shaderSourceDataHash = ResourceTypes::GetHash<ShaderSourceData>();
		if (typeHash != shaderSourceDataHash) return;

		m_QueuedShaders.push_back(callback.m_UUID);
		m_pShaderJobsPool->QueueSingleJob(LoadShaderSourceJob, callback.m_UUID);
	}
	
	void EditorShaderProcessor::AssetUpdatedCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta))
			return;
		const uint32_t typeHash = meta.Hash();
		static const size_t shaderSourceDataHash = ResourceTypes::GetHash<ShaderSourceData>();
		if (typeHash != shaderSourceDataHash) return;

		if (m_QueuedShaders.Contains(callback.m_UUID))
			return;

		const std::filesystem::path cachedShaderSourceFile = GetCompiledShaderPath(callback.m_UUID);
		if (std::filesystem::exists(cachedShaderSourceFile))
			std::filesystem::remove(cachedShaderSourceFile);

		m_QueuedShaders.push_back(callback.m_UUID);
		m_pShaderJobsPool->QueueSingleJob(LoadShaderSourceJob, callback.m_UUID);
	}

	bool EditorShaderProcessor::LoadShaderSourceJob(UUID uuid)
	{
		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(uuid, location))
			return false;

		std::filesystem::path assetPath = ProjectSpace::GetOpenProject()->RootPath();
		assetPath.append("Assets").append(location.Path);
		if (!std::filesystem::exists(assetPath))
		{
			assetPath = location.Path;
		}

		Resource* pResource = Importer::Import(assetPath, nullptr);
		if (!pResource) return false;
		pResource->SetResourceUUID(uuid);
		ShaderSourceData* pShaderSource = static_cast<ShaderSourceData*>(pResource);

		if (m_pLoadedShaderSources.Contains(pShaderSource->GetUUID()))
		{
			m_pLoadedShaderSources.Do(pShaderSource->GetUUID(), [&](ShaderSourceData** val)
			{
				/* Replace */
				delete* val;
				*val = pShaderSource;
			});
		}
		else
		{
			m_pLoadedShaderSources.Set(pShaderSource->GetUUID(), pShaderSource);
		}
		m_pShaderJobsPool->QueueSingleJob(CompileShaderJob, uuid);
		return true;
	}

	bool EditorShaderProcessor::CompileShaderJob(UUID uuid)
	{
		const std::filesystem::path cachedShaderSourceFile = GetCompiledShaderPath(uuid);

		ShaderSourceData* pShaderSource = GetShaderSource(uuid);

		EditorShaderData* pEditorShaderData = nullptr;
		if (std::filesystem::exists(cachedShaderSourceFile))
			LoadCache(uuid, cachedShaderSourceFile);
		else
			m_pLoadedShaderSources.Do(uuid, [&pEditorShaderData, cachedShaderSourceFile](ShaderSourceData* pShaderSource) {
				pEditorShaderData = CompileAndCache(pShaderSource, cachedShaderSourceFile);
			});

		if (pEditorShaderData == nullptr)
		{
			m_QueuedShaders.Erase(uuid);
			return false;
		}

		ProcessReflection(pEditorShaderData);

		const std::string path = EditorApplication::GetInstance()->GetEngine()->GetShaderManager().GetCompiledShaderPath(pEditorShaderData->GetUUID());
		CompileForCurrentPlatform(pEditorShaderData, path);

		FileImportSettings importSettings;
		importSettings.AddNullTerminateAtEnd = true;
		importSettings.Flags = std::ios::ate | std::ios::binary;
		importSettings.m_Extension = "";
		FileData* pCompiledShader = (FileData*)EditorApplication::GetInstance()->GetEngine()->GetLoaderModule<FileData>()->Load(path, importSettings);
		pShaderSource->SetCompiledShader(pCompiledShader);

		m_QueuedShaders.Erase(uuid);
		m_FinishedShaders.push_back(uuid);
		return true;
	}

	std::filesystem::path EditorShaderProcessor::GetCompiledShaderPath(UUID uuid)
	{
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path cachedShaderSourceFile = pProject->CachePath();
		cachedShaderSourceFile.append("ShaderSource").append(std::to_string(uuid));
		return cachedShaderSourceFile;
	}
}
