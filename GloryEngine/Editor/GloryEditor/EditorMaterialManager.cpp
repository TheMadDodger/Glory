#include "EditorMaterialManager.h"
#include "EditorAssetCallbacks.h"
#include "EditorAssetDatabase.h"
#include "Importer.h"
#include "ProjectSpace.h"
#include "MaterialData.h"
#include "MaterialInstanceData.h"
#include "EditorShaderProcessor.h"
#include "ResourceType.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditableResource.h"
#include "Dispatcher.h"
#include "EditorShaderData.h"

#include <Serializers.h>
#include <NodeRef.h>

#include <Engine.h>
#include <AssetDatabase.h>
#include <AssetManager.h>

namespace Glory::Editor
{
	EditorMaterialManager::EditorMaterialManager(Engine* pEngine):
		m_pEngine(pEngine), m_AssetRegisteredCallback(0), m_AssetUpdatedCallback(0),
		m_ShaderCompiledCallback(0), MaterialManager(pEngine)
	{
	}

	EditorMaterialManager::~EditorMaterialManager()
	{
	}

	void EditorMaterialManager::Start()
	{
		m_AssetRegisteredCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetRegistered,
			[this](const AssetCallbackData& callback) { AssetAddedCallback(callback); });
		//m_AssetUpdatedCallback = EditorAssetCallbacks::RegisterCallback(AssetCallbackType::CT_AssetUpdated,
			//[this](const AssetCallbackData& callback) { AssetUpdatedCallback(callback); });

		m_ShaderCompiledCallback = EditorShaderProcessor::ShaderCompiledEventDispatcher().AddListener([this](const ShaderCompiledEvent& e) {
			OnShaderCompiled(e.ShaderID);
		});
	}

	void EditorMaterialManager::Stop()
	{
		EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetRegisteredCallback);
		EditorShaderProcessor::ShaderCompiledEventDispatcher().RemoveListener(m_ShaderCompiledCallback);
		//EditorAssetCallbacks::RemoveCallback(AssetCallbackType::CT_AssetRegistered, m_AssetUpdatedCallback);
	}

	void EditorMaterialManager::LoadIntoMaterial(Utils::YAMLFileRef& file, MaterialData* pMaterial) const
	{
		auto shaders = file["Shaders"];
		ReadShadersInto(shaders, pMaterial);
		auto properties = file["Properties"];
		ReadPropertiesInto(shaders, pMaterial);
	}

	void EditorMaterialManager::AddShaderToMaterial(UUID materialID, UUID shaderID)
	{
		auto itor = m_pMaterialDatas.find(materialID);
		if (itor == m_pMaterialDatas.end()) return;
		itor->second->AddShader(shaderID);
		UpdateMaterial(itor->second);
	}

	MaterialData* EditorMaterialManager::GetMaterial(UUID materialID) const
	{
		auto itor = m_pMaterialDatas.find(materialID);
		if (itor == m_pMaterialDatas.end()) return nullptr;
		return itor->second;
	}

	void EditorMaterialManager::AssetAddedCallback(const AssetCallbackData& callback)
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

		const uint32_t typeHash = meta.Hash();
		static const size_t materialDataHash = ResourceTypes::GetHash<MaterialData>();
		static const size_t materialInstanceDataHash = ResourceTypes::GetHash<MaterialInstanceData>();
		if (typeHash == materialDataHash || typeHash == materialInstanceDataHash)
		{
			//auto itor = m_pMaterialDatas.find(callback.m_UUID);
			//
			//if (itor != m_pMaterialDatas.end())
			//{
			//	EditableResource* pEditableResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(callback.m_UUID);
			//	if (!pEditableResource) return;
			//	YAMLResource<MaterialData>* pEditableMaterial = static_cast<YAMLResource<MaterialData>*>(pEditableResource);
			//	LoadIntoMaterial(**pEditableMaterial, itor->second);
			//}

			/* Immediately import the material, which is fast */
			Resource* pResource = Importer::Import(assetPath, nullptr);
			if (!pResource) return;
			MaterialData* pMaterial = static_cast<MaterialData*>(pResource);
			pMaterial->SetResourceUUID(callback.m_UUID);
			m_pMaterialDatas[callback.m_UUID] = pMaterial;
		}
	}

	void EditorMaterialManager::AssetUpdatedCallback(const AssetCallbackData& callback)
	{
		ResourceMeta meta;
		EditorAssetDatabase::GetAssetMetadata(callback.m_UUID, meta);
		const uint32_t typeHash = meta.Hash();
		static const size_t shaderSourceDataHash = ResourceTypes::GetHash<MaterialData>();
		if (typeHash != shaderSourceDataHash) return;
	}

	void EditorMaterialManager::ReadShadersInto(Utils::NodeValueRef& shaders, MaterialData* pMaterial) const
	{
		pMaterial->RemoveAllShaders();
		if (!shaders.IsSequence()) return;
		for (size_t i = 0; i < shaders.Size(); ++i)
		{
			auto shader = shaders[i];
			const UUID shaderID = shader["UUID"].As<uint64_t>();

			AssetLocation location;
			if (!EditorAssetDatabase::GetAssetLocation(shaderID, location)) continue;

			const std::string_view assetPath = m_pEngine->GetAssetDatabase().GetAssetPath();
			const std::string path = std::string{ assetPath } + '\\' + location.Path;
			pMaterial->AddShader(shaderID);
		}
	}

	void EditorMaterialManager::ReadPropertiesInto(Utils::NodeValueRef& properties, MaterialData* pMaterial, bool clearProperties) const
	{
		if (!properties.IsMap()) return;
		if (clearProperties) pMaterial->ClearProperties();

		for (auto itor = properties.Begin(); itor != properties.End(); ++itor)
		{
			const std::string name = *itor;
			auto prop = properties[name];
			const std::string displayName = prop["DisplayName"].As<std::string>();
			const uint32_t type = prop["TypeHash"].As<uint32_t>();
			auto value = prop["Value"];

			const BasicTypeData* typeData = m_pEngine->GetResourceTypes().GetBasicTypeData(type);

			bool isResource = m_pEngine->GetResourceTypes().IsResource(type);
			if (!isResource)
			{
				pMaterial->AddProperty(displayName, name, type, typeData != nullptr ? typeData->m_Size : 4, 0);
				size_t index = 0;
				pMaterial->GetPropertyInfoIndex(displayName, index);
				const size_t offset = pMaterial->GetPropertyInfoAt(index)->Offset();
				m_pEngine->GetSerializers().DeserializeProperty(pMaterial->GetBufferReference(), type, offset, typeData != nullptr ? typeData->m_Size : 4, value.Node());
			}
			else
			{
				const UUID id = value.As<uint64_t>();
				pMaterial->AddProperty(displayName, name, type, id);
			}
		}
	}

	void EditorMaterialManager::OnShaderCompiled(const UUID& uuid)
	{
		for (auto itor = m_pMaterialDatas.begin(); itor != m_pMaterialDatas.end(); ++itor)
		{
			if (!itor->second->HasShader(uuid)) continue;
			UpdateMaterial(itor->second);
		}
	}

	void EditorMaterialManager::UpdateMaterial(MaterialData* pMaterial)
	{
		EditorApplication* pApplication = EditorApplication::GetInstance();

		pMaterial->ClearProperties();
		for (size_t i = 0; i < pMaterial->ShaderCount(); ++i)
		{
			const UUID shaderID = pMaterial->GetShaderIDAt(i);
			EditorShaderData* pShader = EditorShaderProcessor::GetEditorShader(shaderID);
			if (!pShader) continue;
			pShader->LoadIntoMaterial(pMaterial);
		}

		YAMLResource<MaterialData>* pEditorMaterialData = (YAMLResource<MaterialData>*)pApplication->GetResourceManager().GetEditableResource(pMaterial->GetUUID());
		Utils::YAMLFileRef& file = **pEditorMaterialData;
		ReadPropertiesInto(file["Properties"], pMaterial, false);
		/* Update properties in YAML? */
	}
}
