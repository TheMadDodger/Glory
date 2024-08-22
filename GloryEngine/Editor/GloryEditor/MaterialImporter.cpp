#include "MaterialImporter.h"
#include "EditorAssetDatabase.h"
#include "EditorApplication.h"
#include "EditorMaterialManager.h"
#include "EditorResourceManager.h"

#include <fstream>
#include <AssetManager.h>
#include <AssetDatabase.h>
#include <PropertySerializer.h>

namespace Glory::Editor
{
	MaterialImporter::MaterialImporter()
	{
	}

	MaterialImporter::~MaterialImporter()
	{
	}

	std::string_view MaterialImporter::Name() const
	{
		return "Internal Material Importer";
	}

	bool MaterialImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".gmat") == 0;
	}

	ImportedResource MaterialImporter::LoadResource(const std::filesystem::path& path, void*) const
	{
		MaterialData* pMaterialData = new MaterialData();
		Utils::YAMLFileRef file{ path };
		EditorApplication::GetInstance()->GetMaterialManager().LoadIntoMaterial(file, pMaterialData);
		return ImportedResource{ path, pMaterialData };
	}

	bool MaterialImporter::SaveResource(const std::filesystem::path& path, MaterialData* pResource) const
	{
		Utils::YAMLFileRef file{ path };
		SaveMaterialData(pResource, file.RootNodeRef().ValueRef());
		file.Save();
		return true;
	}

	void MaterialImporter::SaveMaterialData(MaterialData* pMaterialData, Utils::NodeValueRef data) const
	{
		data.SetMap();
		data["Pipeline"].Set(uint64_t(pMaterialData->GetPipelineID(EditorApplication::GetInstance()->GetMaterialManager())));
		WritePropertyData(data, pMaterialData);
	}

	void MaterialImporter::WritePropertyData(Utils::NodeValueRef data, MaterialData* pMaterialData) const
	{
		MaterialManager& manager = EditorApplication::GetInstance()->GetEngine()->GetMaterialManager();

		auto properties = data["Properties"];

		size_t resourceIndex = 0;
		for (size_t i = 0; i < pMaterialData->PropertyInfoCount(manager); i++)
		{
			properties.PushBack(YAML::Node(YAML::NodeType::Map));
			auto property = properties[i];

			MaterialPropertyInfo* pPropertyInfo = pMaterialData->GetPropertyInfoAt(manager, i);
			property["DisplayName"].Set(pPropertyInfo->DisplayName());
			property["ShaderName"].Set(pPropertyInfo->ShaderName());
			property["TypeHash"].Set(pPropertyInfo->TypeHash());

			bool isResource = EditorApplication::GetInstance()->GetEngine()->GetResourceTypes().IsResource(pPropertyInfo->TypeHash());
			if (!isResource)
			{
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().SerializeProperty("Value", pMaterialData->GetBufferReference(manager), pPropertyInfo->TypeHash(), pPropertyInfo->Offset(), pPropertyInfo->Size(), property["Value"]);
			}
			else
			{
				size_t index = pMaterialData->GetPropertyIndexFromResourceIndex(manager, resourceIndex);
				++resourceIndex;
				const uint64_t uuid = pMaterialData->GetResourceUUIDPointer(manager, index)->AssetUUID();
				property["Value"].Set(uuid);
			}
		}
	}

	void MaterialImporter::Initialize()
	{
	}

	void MaterialImporter::Cleanup()
	{
	}
}
