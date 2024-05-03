#include "EditorPipelineManager.h"
#include "EditorAssetCallbacks.h"
#include "EditorAssetDatabase.h"
#include "EditorResourceManager.h"
#include "ProjectSpace.h"
#include "EditorApplication.h"
#include "EditableResource.h"
#include "EditorShaderData.h"
#include "EditorShaderProcessor.h"
#include "Dispatcher.h"
#include "Importer.h"

#include <Engine.h>
#include <AssetManager.h>

#include <PipelineData.h>

namespace Glory::Editor
{
	EditorPipelineManager::EditorPipelineManager(Engine* pEngine): PipelineManager(pEngine)
	{
	}

	EditorPipelineManager::~EditorPipelineManager()
	{
		m_pEngine = nullptr;
	}

	void EditorPipelineManager::Initialize()
	{
		m_AssetRegisteredCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered,
			[this](const AssetCallbackData& callback) { AssetAddedCallback(callback); });
		//m_AssetUpdatedCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetUpdated,
			//[this](const AssetCallbackData& callback) { AssetUpdatedCallback(callback); });

		m_ShaderCompiledCallback = EditorShaderProcessor::ShaderCompiledEventDispatcher().AddListener([this](const ShaderCompiledEvent& e) {
			OnShaderCompiled(e.ShaderID);
		});
	}

	void EditorPipelineManager::Cleanup()
	{
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetRegisteredCallback);
		EditorShaderProcessor::ShaderCompiledEventDispatcher().RemoveListener(m_ShaderCompiledCallback);
		//EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetUpdatedCallback);
	}

	void EditorPipelineManager::AddShaderToPipeline(UUID pipelineID, UUID shaderID)
	{
		auto itor = m_pPipelineDatas.find(pipelineID);
		if (itor == m_pPipelineDatas.end()) return;
		itor->second->AddShader(shaderID);
		YAMLResource<PipelineData>* pMaterialData = static_cast<YAMLResource<PipelineData>*>(
			EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(pipelineID));
		Utils::YAMLFileRef& file = **pMaterialData;
		auto shaders = file["Shaders"];
		shaders[shaders.Size()].Set(uint64_t(shaderID));
		UpdatePipeline(itor->second);
	}

	void EditorPipelineManager::RemoveShaderFromPipeline(UUID pipelineID, size_t index)
	{
		auto itor = m_pPipelineDatas.find(pipelineID);
		if (itor == m_pPipelineDatas.end()) return;
		itor->second->RemoveShaderAt(index);
		YAMLResource<PipelineData>* pMaterialData = static_cast<YAMLResource<PipelineData>*>(
			EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(pipelineID));
		Utils::YAMLFileRef& file = **pMaterialData;
		auto shaders = file["Shaders"];
		shaders.Remove(index);
		UpdatePipeline(itor->second);
	}

	PipelineData* EditorPipelineManager::GetPipelineData(UUID materialID) const
	{
		auto itor = m_pPipelineDatas.find(materialID);
		if (itor == m_pPipelineDatas.end()) return nullptr;
		return itor->second;
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
			m_pPipelineDatas[callback.m_UUID] = pPipeline;

			UpdatePipeline(pPipeline);

			const uint64_t pipelineType = pPipeline->UsesTextures() ? uint64_t(pPipeline->Type()) << 32 : uint64_t(pPipeline->Type());
			if (m_DefaultPipelinesMap.find(pipelineType) == m_DefaultPipelinesMap.end())
				m_DefaultPipelinesMap.emplace(pipelineType, callback.m_UUID);
		}
	}

	void EditorPipelineManager::AssetUpdatedCallback(const AssetCallbackData& callback)
	{
	}

	void EditorPipelineManager::OnShaderCompiled(const UUID& uuid)
	{
		for (auto itor = m_pPipelineDatas.begin(); itor != m_pPipelineDatas.end(); ++itor)
		{
			if (!itor->second->HasShader(uuid)) continue;
			UpdatePipeline(itor->second);
		}
	}

	void EditorPipelineManager::UpdatePipeline(PipelineData* pPipeline)
	{
		EditorApplication* pApplication = EditorApplication::GetInstance();

		pPipeline->ClearProperties();
		for (size_t i = 0; i < pPipeline->ShaderCount(); ++i)
		{
			const UUID shaderID = pPipeline->ShaderID(i);
			EditorShaderData* pShader = EditorShaderProcessor::GetEditorShader(shaderID);
			if (!pShader) continue;
			pShader->LoadIntoPipeline(pPipeline);
		}
	}

	void EditorPipelineManager::LoadIntoPipeline(Utils::YAMLFileRef& file, PipelineData* pPipeline) const
	{
		pPipeline->SetPipelineType(file["Type"].AsEnum<PipelineType>());
		pPipeline->RemoveAllShaders();
		if (!file["Shaders"].IsSequence()) return;
		for (size_t i = 0; i < file["Shaders"].Size(); ++i)
		{
			auto shader = file["Shaders"][i];
			const UUID shaderID = shader.As<uint64_t>();
			pPipeline->AddShader(shaderID);
		}
	}
}
