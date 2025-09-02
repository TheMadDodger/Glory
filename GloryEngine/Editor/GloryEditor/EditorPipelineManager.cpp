#include "EditorPipelineManager.h"
#include "EditorAssetCallbacks.h"
#include "EditorAssetDatabase.h"
#include "EditorResourceManager.h"
#include "ProjectSpace.h"
#include "EditorApplication.h"
#include "EditableResource.h"
#include "EditorShaderData.h"
#include "Dispatcher.h"
#include "Importer.h"
#include "EditorPipeline.h"

#include <ShaderSourceData.h>
#include <Engine.h>
#include <Debug.h>
#include <AssetManager.h>
#include <BinaryStream.h>
#include <AssetArchive.h>
#include <JobManager.h>
#include <PipelineData.h>

#include <shaderc/shaderc.hpp>

namespace Glory::Editor
{
	ThreadedVector<UUID> EditorPipelineManager::m_QueuedPipelines;
	ThreadedVector<EditorPipeline*> EditorPipelineManager::m_FinishedPipelines;
	std::mutex EditorPipelineManager::m_WaitMutex;
	std::condition_variable EditorPipelineManager::m_WaitCondition;
	ThreadedUMap<UUID, ShaderSourceData*> EditorPipelineManager::m_pLoadedShaderSources;
	std::vector<ShaderSourceData*> EditorPipelineManager::m_pOutdatedShaders;

	std::map<ShaderType, shaderc_shader_kind> ShaderTypeToKindOne = {
		{ ShaderType::ST_Compute, shaderc_shader_kind::shaderc_compute_shader },
		{ ShaderType::ST_Fragment, shaderc_shader_kind::shaderc_fragment_shader },
		{ ShaderType::ST_Geomtery, shaderc_shader_kind::shaderc_geometry_shader },
		{ ShaderType::ST_TessControl, shaderc_shader_kind::shaderc_tess_control_shader },
		{ ShaderType::ST_TessEval, shaderc_shader_kind::shaderc_tess_evaluation_shader },
		{ ShaderType::ST_Vertex, shaderc_shader_kind::shaderc_vertex_shader },
	};

	std::map<spirv_cross::SPIRType::BaseType, std::vector<uint32_t>> SpirBaseTypeToHashOne = {
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

	EditorPipelineManager::EditorPipelineManager(Engine* pEngine): PipelineManager(pEngine), m_pPipelineJobsPool(nullptr)
	{
	}

	EditorPipelineManager::~EditorPipelineManager()
	{
		m_pLoadedShaderSources.ForEachClear([](ShaderSourceData* pShader) { delete pShader; });
		for (ShaderSourceData* pShader : m_pOutdatedShaders)
			delete pShader;
		m_pOutdatedShaders.clear();
		m_pEngine = nullptr;
	}

	void EditorPipelineManager::Initialize()
	{
		m_pEngine->GetResourceTypes().RegisterResource<ShaderSourceData>("");
		m_pEngine->GetResourceTypes().RegisterResource<EditorPipeline>("");
		m_AssetRegisteredCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered,
			[this](const AssetCallbackData& callback) { AssetAddedCallback(callback); });
		m_AssetUpdatedCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetUpdated,
			[this](const AssetCallbackData& callback) { AssetUpdatedCallback(callback); });

		m_pPipelineJobsPool = Jobs::JobManager::Run<bool, UUID>();
	}

	void EditorPipelineManager::Cleanup()
	{
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetRegisteredCallback);
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetUpdated, m_AssetUpdatedCallback);
	}

	void EditorPipelineManager::AddShaderToPipeline(UUID pipelineID, UUID shaderID)
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(pipelineID);
		if (!pResource) return;
		PipelineData* pPipeline = static_cast<PipelineData*>(pResource);
		pPipeline->AddShader(shaderID);
		YAMLResource<PipelineData>* pPipelineData = static_cast<YAMLResource<PipelineData>*>(
			EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(pipelineID));
		Utils::YAMLFileRef& file = **pPipelineData;
		auto shaders = file["Shaders"];
		shaders[shaders.Size()].Set(uint64_t(shaderID));
		DeletePipelineCache(pipelineID);
		QueueCompileJob(pipelineID);
	}

	void EditorPipelineManager::RemoveShaderFromPipeline(UUID pipelineID, size_t index)
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(pipelineID);
		if (!pResource) return;
		PipelineData* pPipeline = static_cast<PipelineData*>(pResource);
		pPipeline->RemoveShaderAt(index);
		YAMLResource<PipelineData>* pMaterialData = static_cast<YAMLResource<PipelineData>*>(
			EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(pipelineID));
		Utils::YAMLFileRef& file = **pMaterialData;
		auto shaders = file["Shaders"];
		shaders.Remove(index);
		DeletePipelineCache(pipelineID);
		QueueCompileJob(pipelineID);
	}

	void EditorPipelineManager::SetPipelineFeatureEnabled(UUID pipelineID, std::string_view feature, bool enable)
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(pipelineID);
		if (!pResource) return;
		PipelineData* pPipeline = static_cast<PipelineData*>(pResource);
		const size_t index = pPipeline->FeatureIndex(feature);
		pPipeline->SetFeatureEnabled(index, enable);
		DeletePipelineCache(pipelineID);
		QueueCompileJob(pipelineID);
	}

	PipelineData* EditorPipelineManager::GetPipelineData(UUID pipelineID) const
	{
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(pipelineID);
		if (!pResource) return nullptr;
		return static_cast<PipelineData*>(pResource);
	}

	const std::vector<FileData>& EditorPipelineManager::GetPipelineCompiledShaders(UUID pipelineID) const
	{
		for (size_t i = 0; i < m_Pipelines.size(); ++i)
		{
			if (m_Pipelines[i] != pipelineID) continue;
			return m_CompiledShaders[i];
		}

		return {};
	}

	const std::vector<ShaderType>& EditorPipelineManager::GetPipelineShaderTypes(UUID pipelineID) const
	{
		for (size_t i = 0; i < m_Pipelines.size(); ++i)
		{
			if (m_Pipelines[i] != pipelineID) continue;
			return m_ShaderTypes[i];
		}

		return {};
	}

	void EditorPipelineManager::AddShader(FileData* pShader)
	{
		throw new std::exception("Adding extrenally compiled shaders is not allowed in the editor!");
	}

	UUID EditorPipelineManager::FindPipeline(PipelineType type, bool useTextures) const
	{
		for (auto pipelineID: m_Pipelines)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(pipelineID);
			if (!pResource) continue;
			PipelineData* pPipeline = static_cast<PipelineData*>(pResource);
			if (pPipeline->Type() == type && pPipeline->UsesTextures() == useTextures)
				return pipelineID;
		}
		return 0;
	}

	EditorPipelineManager::PipelineUpdateDispatcher& EditorPipelineManager::PipelineUpdateEvents()
	{
		static EditorPipelineManager::PipelineUpdateDispatcher dispatcher;
		return dispatcher;
	}

	TextureType EditorPipelineManager::ShaderNameToTextureType(std::string_view name)
	{
		/* Hardcoded solution for texSampler */
		if (name.compare("texSampler") == 0)
		{
			return TextureType::TT_BaseColor;
		}

		const size_t samplerNameIndex = name.find("Sampler");
		if (samplerNameIndex != std::string::npos)
			name = name.substr(0, samplerNameIndex);
		/* Skip the first letter to avoid case mismatch */
		name = name.substr(1);

		std::vector<TextureType> contenders;
		for (uint32_t i = Enum<TextureType>().NumValues(); i > 0; --i)
		{
			const TextureType textureType = TextureType(i - 1);
			std::string valueStr;
			Enum<TextureType>().ToString(textureType, valueStr);
			if (valueStr.find(name) == std::string::npos) continue;
			contenders.push_back(textureType);
		}

		if (contenders.size() == 0)
			return TT_Unknown;

		if (contenders.size() == 1)
			return contenders[0];

		size_t bestContender = 0;
		size_t bestContenderScore = 0;
		for (size_t i = 0; i < contenders.size(); ++i)
		{
			const TextureType textureType = contenders[i];
			std::string valueStr;
			Enum<TextureType>().ToString(textureType, valueStr);
			const size_t startPos = valueStr.find(name);
			const size_t endPos = startPos + name.size();
			const size_t score = valueStr.size() - endPos + startPos - 1;
			if (i == 0)
			{
				bestContender = 0;
				bestContenderScore = score;
				continue;
			}

			if (score < bestContenderScore)
			{
				bestContenderScore = score;
				bestContender = i;
			}
		}
		return contenders[bestContender];
	}

	ShaderSourceData* EditorPipelineManager::GetShaderSource(UUID shaderID)
	{
		if (!m_pLoadedShaderSources.Contains(shaderID)) return nullptr;
		return m_pLoadedShaderSources.at(shaderID);
	}

	void EditorPipelineManager::RunCallbacks()
	{
		m_FinishedPipelines.ForEachClear([this](EditorPipeline* pEditorPipeline) {
			PipelineData* pPipeline = GetPipelineData(pEditorPipeline->GetUUID());
			if (!pPipeline) return;
			UpdatePipeline(pPipeline, pEditorPipeline);
			delete pEditorPipeline;
		});

		if (!IsBusy())
		{
			for (ShaderSourceData* pShader : m_pOutdatedShaders)
				delete pShader;
			m_pOutdatedShaders.clear();
		}
	}

	void EditorPipelineManager::AssetAddedCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta))
			return;
		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(callback.m_UUID, location))
			return;

		std::filesystem::path assetPath = ProjectSpace::GetOpenProject()->RootPath();
		assetPath.append("Assets").append(location.Path);
		if (!std::filesystem::exists(assetPath))
		{
			assetPath = location.Path;
		}

		EditorResourceManager& resourceManager = EditorApplication::GetInstance()->GetResourceManager();

		const uint32_t typeHash = meta.Hash();
		static const size_t pipelineDataHash = ResourceTypes::GetHash<PipelineData>();
		if (typeHash == pipelineDataHash)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(callback.m_UUID);
			if (!pResource)
			{
				PipelineData* pPipelineData = new PipelineData();
				pResource = pPipelineData;
				pResource->SetResourceUUID(callback.m_UUID);
				YAMLResource<PipelineData>* pPipeline = static_cast<YAMLResource<PipelineData>*>(resourceManager.GetEditableResource(callback.m_UUID));
				if (!pPipeline)
				{
					delete pPipelineData;
					return;
				}

				LoadIntoPipeline(**pPipeline, pPipelineData);
				m_pEngine->GetAssetManager().AddLoadedResource(pResource);
			}

			PipelineData* pPipeline = static_cast<PipelineData*>(pResource);
			pPipeline->SetResourceUUID(callback.m_UUID);
			if (std::find(m_Pipelines.begin(), m_Pipelines.end(), callback.m_UUID) == m_Pipelines.end())
			{
				m_Pipelines.emplace_back(callback.m_UUID);
				m_CompiledShaders.emplace_back();
				m_ShaderTypes.emplace_back();
			}

			EditorPipeline* pEditorPipeline = CompilePipelineForEditor(pPipeline);
			UpdatePipeline(pPipeline, pEditorPipeline);
		}
	}

	void EditorPipelineManager::AssetUpdatedCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta))
			return;
		const uint32_t typeHash = meta.Hash();
		static const size_t shaderSourceDataHash = ResourceTypes::GetHash<ShaderSourceData>();
		if (typeHash != shaderSourceDataHash) return;

		/* Mark loaded shader as outdated */
		m_pLoadedShaderSources.DoErase(callback.m_UUID,
			[this](ShaderSourceData** pShader) { m_pOutdatedShaders.push_back(*pShader); });

		/* Recompile pipelines that use this shader */
		for (const UUID pipelineID : m_Pipelines)
		{
			Resource* pResource = m_pEngine->GetAssetManager().FindResource(pipelineID);
			if (!pResource) continue;
			PipelineData* pPipeline = static_cast<PipelineData*>(pResource);
			if (!pPipeline->HasShader(callback.m_UUID)) continue;
			DeletePipelineCache(pipelineID);
			QueueCompileJob(pipelineID);
		}
	}

	void EditorPipelineManager::UpdatePipeline(PipelineData* pPipeline, EditorPipeline* pEditorPipeline)
	{
		if (!pEditorPipeline) return;

		EditorApplication* pApplication = EditorApplication::GetInstance();

		pPipeline->ClearProperties();
		pPipeline->ClearFeatures();
		for (size_t i = 0; i < pPipeline->ShaderCount(); ++i)
		{
			const UUID shaderID = pPipeline->ShaderID(i);
			const EditorShaderData& shader = pEditorPipeline->m_EditorShaderDatas[i];
			shader.LoadIntoPipeline(pPipeline);

			for (size_t i = 0; i < shader.m_Features.size(); ++i)
			{
				const std::string_view feature = shader.m_Features[i];
				pPipeline->AddFeature(feature, true);
			}
		}

		for (size_t i = 0; i < m_Pipelines.size(); ++i)
		{
			if (m_Pipelines[i] != pPipeline->GetUUID()) continue;
			m_CompiledShaders[i].clear();
			m_ShaderTypes[i].clear();
			m_CompiledShaders[i].reserve(pEditorPipeline->m_EditorPlatformShaders.size());
			m_ShaderTypes[i].reserve(pEditorPipeline->m_EditorPlatformShaders.size());
			for (size_t j = 0; j < pEditorPipeline->m_EditorPlatformShaders.size(); ++j)
			{
				const size_t index = m_CompiledShaders[i].size();
				m_CompiledShaders[i].push_back(FileData(pEditorPipeline->m_EditorPlatformShaders[j]));
				m_ShaderTypes[i].push_back(pEditorPipeline->m_EditorShaderDatas[j].m_ShaderType);
				m_CompiledShaders[i][index].SetMetaData(PipelineShaderMetaData{ m_Pipelines[i], m_ShaderTypes[i][index] });
			}
		}

		pPipeline->SetDirty(true);
		PipelineUpdateEvents().Dispatch({ pPipeline });
	}

	void EditorPipelineManager::LoadIntoPipeline(Utils::YAMLFileRef& file, PipelineData* pPipeline) const
	{
		pPipeline->SetPipelineType(file["Type"].AsEnum<PipelineType>());
		pPipeline->RemoveAllShaders();
		for (size_t i = 0; i < file["Shaders"].IsSequence() && file["Shaders"].Size(); ++i)
		{
			auto shader = file["Shaders"][i];
			const UUID shaderID = shader.As<uint64_t>();
			pPipeline->AddShader(shaderID);
		}
	}

	bool EditorPipelineManager::IsCacheOutdated(const std::filesystem::path& cachePath, ShaderSourceData* shaderSource)
	{
		const auto time = std::filesystem::last_write_time(cachePath);
		const uint64_t cacheWriteTime = std::chrono::duration_cast<std::chrono::seconds>(
			time.time_since_epoch()).count();
		return cacheWriteTime < shaderSource->TimeSinceLastWrite();
	}

	EditorPipeline* EditorPipelineManager::CompilePipelineForEditor(PipelineData* pPipeline)
	{
		EditorRenderImpl* pEditorRenderer = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();
		Debug& debug = m_pEngine->GetDebug();

		static const size_t featureLength = strlen("FEATURE_");

		std::vector<EditorShaderData> compiledShaders(pPipeline->ShaderCount());

		/* Check if cache is available and not outdated */
		const std::filesystem::path cachePath = GetCompiledPipelineCachePath(pPipeline->GetUUID());
		bool validCacheAvailable = std::filesystem::exists(cachePath);
		for (size_t i = 0; i < pPipeline->ShaderCount() && validCacheAvailable; ++i)
		{
			ShaderSourceData* pShaderSource = LoadOriginalShader(pPipeline->ShaderID(i));
			if (!pShaderSource) continue;
			if (!IsCacheOutdated(cachePath, pShaderSource)) continue;
			validCacheAvailable = false;
			break;
		}

		if (validCacheAvailable)
		{
			/* Load cache */
			BinaryFileStream file{ cachePath, true, false };
			AssetArchive archive{ &file, AssetArchiveFlags::Read };
			/* If the cache is from an older version it is likely invalid */
			if (archive.VerifyVersion())
			{
				archive.Deserialize(m_pEngine);
				if (archive.Size())
				{
					Resource* pResource = archive.Get(m_pEngine, 0);
					return static_cast<EditorPipeline*>(pResource);
				}
			}
			validCacheAvailable = false;
		}

		for (size_t i = 0; i < pPipeline->ShaderCount(); ++i)
		{
			ShaderSourceData* pShaderSource = LoadOriginalShader(pPipeline->ShaderID(i));
			if (!pShaderSource) continue;

			const ShaderType shaderType = pShaderSource->GetShaderType();
			if (ShaderTypeToKindOne.find(shaderType) == ShaderTypeToKindOne.end())
			{
				debug.LogError("Shader " + pShaderSource->Name() + " compilation failed due to unknown shader type.");
				return nullptr;
			}

			shaderc::CompileOptions options{};

			for (size_t j = 0; j < pShaderSource->FeatureCount(); ++j)
			{
				const std::string_view name = pShaderSource->Feature(j);
				compiledShaders[i].m_Features.emplace_back(name);
				const size_t index = pPipeline->FeatureIndex(name);
				const bool enabled = pPipeline->FeatureEnabled(index);
				if (!enabled) continue;
				const std::string_view actualName = name.substr(featureLength);
				const std::string definition = "WITH_" + std::string(actualName);
				options.AddMacroDefinition(definition);
			}

			if (pEditorRenderer->PushConstantsSupported())
				options.AddMacroDefinition("PUSH_CONSTANTS");

			compiledShaders[i].m_ShaderType = shaderType;
			shaderc::Compiler compiler;
			shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(pShaderSource->Data(),
				pShaderSource->Size(), ShaderTypeToKindOne.at(shaderType), pShaderSource->Name().data(),
				options);

			const size_t errors = result.GetNumErrors();
			const size_t warnings = result.GetNumWarnings();

			if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
			{
				debug.LogError("Shader " + pShaderSource->Name() + " compilation failed with " + std::to_string(errors) + " errors and " + std::to_string(warnings) + " warnings.");
				debug.LogError(result.GetErrorMessage());
				return nullptr;
			}
			else
				debug.LogInfo("Shader " + pShaderSource->Name() + " compilation succeeded with " + std::to_string(errors) + " errors and " + std::to_string(warnings) + " warnings.");

			if (warnings > 0)
				debug.LogWarning(result.GetErrorMessage());

			compiledShaders[i].m_ShaderData.assign(result.begin(), result.end());
			ProcessReflection(&compiledShaders[i]);
		}

		/* Cross compile for editor platform */
		EditorPipeline* editorPipeline = new EditorPipeline(std::move(compiledShaders));
		editorPipeline->SetResourceUUID(pPipeline->GetUUID());
		CompileForEditorPlatform(editorPipeline);

		/* Cache pipeline */
		BinaryFileStream file{ cachePath };
		AssetArchive archive{ &file, AssetArchiveFlags::WriteNew };
		archive.Serialize(editorPipeline);
		return editorPipeline;
	}

	void EditorPipelineManager::CompileForEditorPlatform(EditorPipeline* pEditorPipeline)
	{
		EditorRenderImpl* pEditorRenderer = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();

		pEditorPipeline->m_EditorPlatformShaders.clear();
		pEditorPipeline->m_EditorPlatformShaders.resize(pEditorPipeline->m_EditorShaderDatas.size());
		for (size_t i = 0; i < pEditorPipeline->m_EditorShaderDatas.size(); ++i)
		{
			const EditorShaderData& editorShader = pEditorPipeline->m_EditorShaderDatas[i];
			pEditorRenderer->CompileShaderForEditor(editorShader, pEditorPipeline->m_EditorPlatformShaders[i]);
		}
	}

	std::filesystem::path EditorPipelineManager::GetCompiledPipelineCachePath(UUID uuid)
	{
		EditorRenderImpl* pEditorRenderer = EditorApplication::GetInstance()->GetEditorPlatform().GetRenderImpl();
		ProjectSpace* pProject = ProjectSpace::GetOpenProject();
		std::filesystem::path cachedShaderSourceFile = pProject->CachePath();
		cachedShaderSourceFile.append("CompiledPipelines").append(pEditorRenderer->ShadingLanguage());
		if (!std::filesystem::exists(cachedShaderSourceFile))
			std::filesystem::create_directories(cachedShaderSourceFile);
		cachedShaderSourceFile.append(std::to_string(uuid));
		return cachedShaderSourceFile;
	}

	ShaderSourceData* EditorPipelineManager::LoadOriginalShader(UUID uuid)
	{
		if (m_pLoadedShaderSources.Contains(uuid))
			return m_pLoadedShaderSources.at(uuid);

		AssetLocation location;
		if (!EditorAssetDatabase::GetAssetLocation(uuid, location))
			return nullptr;

		std::filesystem::path assetPath = ProjectSpace::GetOpenProject()->RootPath();
		assetPath.append("Assets").append(location.Path);
		if (!std::filesystem::exists(assetPath))
		{
			assetPath = location.Path;
		}

		ImportedResource resource = Importer::Import(assetPath, nullptr);
		if (!resource) return nullptr;
		resource->SetResourceUUID(uuid);
		ShaderSourceData* pShaderSource = static_cast<ShaderSourceData*>(*resource);
		const auto time = std::filesystem::last_write_time(assetPath);
		pShaderSource->TimeSinceLastWrite() = std::chrono::duration_cast<std::chrono::seconds>(
			time.time_since_epoch()).count();

		m_pLoadedShaderSources.Set(pShaderSource->GetUUID(), pShaderSource);

		return pShaderSource;
	}

	void EditorPipelineManager::ProcessReflection(EditorShaderData* pEditorShader)
	{
		spirv_cross::Compiler compiler(pEditorShader->Data(), pEditorShader->Size());
		compiler.compile();
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		for (size_t i = 0; i < resources.sampled_images.size(); ++i)
		{
			spirv_cross::Resource sampler = resources.sampled_images[i];
			const spirv_cross::SPIRType& type = compiler.get_type(sampler.type_id);
			ImageType imageType = ImageType::IT_UNDEFINED;
			const bool isArray = type.image.arrayed;
			switch (type.image.dim)
			{
				case spv::Dim::Dim1D:
					imageType = isArray ? ImageType::IT_1DArray : ImageType::IT_1D;
					break;
				case spv::Dim::Dim2D:
					imageType = isArray ? ImageType::IT_2DArray : ImageType::IT_2D;
					break;
				case spv::Dim::Dim3D:
					imageType = ImageType::IT_3D;
					break;
				case spv::Dim::DimCube:
					imageType = isArray ? ImageType::IT_CubeArray : ImageType::IT_Cube;
					break;
			default:
				continue;
			}
			pEditorShader->m_SamplerNames.push_back(sampler.name);
			pEditorShader->m_SamplerTypes.push_back(imageType);
		}

		for (size_t i = 0; i < resources.storage_buffers.size(); ++i)
		{
			const spirv_cross::Resource storageBuffer = resources.storage_buffers[i];
			pEditorShader->m_StorageBuffers.push_back(storageBuffer.name);
			if (storageBuffer.name != "MaterialSSBO") continue;
			const spirv_cross::SPIRType& baseType = compiler.get_type(storageBuffer.base_type_id);
			const spirv_cross::SPIRType& materialArrayType = compiler.get_type(baseType.member_types[0]);
			if (baseType.basetype != spirv_cross::SPIRType::Struct) break;

			for (uint32_t j = 0; j < (uint32_t)materialArrayType.member_types.size(); ++j)
			{
				spirv_cross::TypeID memberTypeID = materialArrayType.member_types[j];
				const spirv_cross::SPIRType& memberType = compiler.get_type(memberTypeID);
				const std::string& name = compiler.get_member_name(materialArrayType.self, j);
				const uint32_t hash = memberType.vecsize - 1 < SpirBaseTypeToHashOne[memberType.basetype].size() ?
					SpirBaseTypeToHashOne[memberType.basetype][memberType.vecsize - 1] : 0;
				pEditorShader->m_PropertyInfos.push_back(EditorShaderData::PropertyInfo(name, hash));
			}
		}

		for (size_t i = 0; i < resources.uniform_buffers.size(); ++i)
		{
			const spirv_cross::Resource uniformBuffer = resources.uniform_buffers[i];
			pEditorShader->m_UniformBuffers.push_back(uniformBuffer.name);
		}
	}

	bool EditorPipelineManager::IsBusy()
	{
		return m_QueuedPipelines.Size() != 0;
	}

	void EditorPipelineManager::WaitIdle()
	{
		std::unique_lock<std::mutex> lock(m_WaitMutex);
		m_WaitCondition.wait(lock, [&]() { return m_QueuedPipelines.Size() == 0; });
	}

	void EditorPipelineManager::QueueCompileJob(UUID pipelineID)
	{
		if (m_QueuedPipelines.Contains(pipelineID)) return;
		m_QueuedPipelines.push_back(pipelineID);
		m_pPipelineJobsPool->QueueSingleJob([this](UUID id) { return CompilePipelineJob(id); }, pipelineID);
	}

	bool EditorPipelineManager::CompilePipelineJob(UUID pipelineID)
	{
		PipelineData* pPipeline = GetPipelineData(pipelineID);
		if (!pPipeline)
		{
			m_QueuedPipelines.Erase(pipelineID);
			return false;
		}
		EditorPipeline* pEditorPipeline = CompilePipelineForEditor(pPipeline);
		if (!pEditorPipeline)
		{
			m_QueuedPipelines.Erase(pipelineID);
			return false;
		}
		m_QueuedPipelines.Erase(pipelineID);
		m_FinishedPipelines.push_back(pEditorPipeline);
		return true;
	}

	void EditorPipelineManager::DeletePipelineCache(UUID pipelineID)
	{
		const std::filesystem::path path = GetCompiledPipelineCachePath(pipelineID);
		std::filesystem::remove(path);
	}
}
