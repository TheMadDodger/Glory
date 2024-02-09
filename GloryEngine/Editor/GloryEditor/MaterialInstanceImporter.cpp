#include "MaterialInstanceImporter.h"
#include "EditorApplication.h"
#include "MaterialManager.h"

#include <fstream>
#include <AssetManager.h>
#include <PropertySerializer.h>

namespace Glory::Editor
{
	MaterialInstanceImporter::MaterialInstanceImporter()
	{
	}

	MaterialInstanceImporter::~MaterialInstanceImporter()
	{
	}

	std::string_view MaterialInstanceImporter::Name() const
	{
		return "Internal Material Instance Importer";
	}

	bool MaterialInstanceImporter::SupportsExtension(const std::filesystem::path& extension) const
	{
		return extension.compare(".gminst") == 0;
	}

	MaterialInstanceData* MaterialInstanceImporter::LoadResource(const std::filesystem::path& path) const
	{
		Utils::YAMLFileRef file{ path };
		file.Load();
		return LoadMaterialInstanceData(file);
	}

	bool MaterialInstanceImporter::SaveResource(const std::filesystem::path& path, MaterialInstanceData* pResource) const
	{
		MaterialInstanceData* pMaterialInstance = dynamic_cast<MaterialInstanceData*>(pResource);
		YAML::Emitter out;
		out << YAML::BeginMap;
		SaveMaterialInstanceData(pMaterialInstance, out);
		out << YAML::EndMap;
		std::ofstream outStream(path);
		outStream << out.c_str();
		outStream.close();
		return true;
	}

	MaterialInstanceData* MaterialInstanceImporter::LoadMaterialInstanceData(Utils::YAMLFileRef& file) const
	{
		const UUID baseMaterial = file["BaseMaterial"].As<uint64_t>();
		MaterialInstanceData* pMaterialInstanceData = new MaterialInstanceData(baseMaterial);
		ReadPropertyOverrides(file, pMaterialInstanceData);
		return pMaterialInstanceData;
	}

	void MaterialInstanceImporter::SaveMaterialInstanceData(MaterialInstanceData* pMaterialData, YAML::Emitter& out) const
	{
		const UUID baseMaterial = pMaterialData->BaseMaterialID();
		YAML_WRITE(out, BaseMaterial, baseMaterial);

		MaterialManager& manager = EditorApplication::GetInstance()->GetEngine()->GetMaterialManager();

		out << YAML::Key << "Overrides";
		out << YAML::Value << YAML::BeginSeq;
		for (size_t i = 0; i < pMaterialData->PropertyInfoCount(manager); ++i)
		{
			const MaterialPropertyInfo* pInfo = pMaterialData->GetPropertyInfoAt(manager, i);
			size_t propertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(manager, pInfo->DisplayName(), propertyIndex)) continue;
			const MaterialPropertyInfo* propertyInfo = pMaterialData->GetPropertyInfoAt(manager, propertyIndex);
			if (!pMaterialData->IsPropertyOverriden(i)) continue;

			out << YAML::BeginMap;
			YAML_WRITE(out, DisplayName, pInfo->DisplayName());
			if (!propertyInfo->IsResource())
			{
				const uint32_t typeHash = propertyInfo->TypeHash();
				const size_t offset = propertyInfo->Offset();
				const size_t size = propertyInfo->Size();
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().SerializeProperty("Value", pMaterialData->GetBufferReference(manager), typeHash, offset, size, out);
			}
			else
			{
				const size_t resourceIndex = propertyInfo->Offset();
				const size_t index = pMaterialData->GetPropertyIndexFromResourceIndex(manager, resourceIndex);
				const UUID uuid = pMaterialData->GetResourceUUIDPointer(manager, index)->AssetUUID();
				out << YAML::Key << "Value" << YAML::Value << uuid;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
	}

	void MaterialInstanceImporter::ReadPropertyOverrides(Utils::YAMLFileRef& file, MaterialInstanceData* pMaterialData) const
	{
		MaterialManager& manager = EditorApplication::GetInstance()->GetEngine()->GetMaterialManager();

		MaterialData* baseMaterial = manager.GetMaterial(pMaterialData->BaseMaterialID());
		if (!baseMaterial) return;
		pMaterialData->Resize(manager, baseMaterial);

		Utils::NodeValueRef propertiesNode = file["Overrides"];
		if (!propertiesNode.IsMap()) return;

		for (auto itor = propertiesNode.Begin(); itor != propertiesNode.End(); ++itor)
		{
			const std::string displayName = *itor;
			auto prop = propertiesNode[displayName];
			const bool enable = prop["Enable"].As<bool>();

			size_t propertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(manager, displayName, propertyIndex)) continue;
			if (enable) pMaterialData->EnableProperty(propertyIndex);

			MaterialPropertyInfo* propertyInfo = pMaterialData->GetPropertyInfoAt(manager, propertyIndex);

			YAML::Node value = prop["Value"].Node();

			if (!propertyInfo->IsResource())
			{
				const uint32_t typeHash = propertyInfo->TypeHash();
				const size_t offset = propertyInfo->Offset();
				const size_t size = propertyInfo->Size();
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().DeserializeProperty(pMaterialData->GetBufferReference(manager), typeHash, offset, size, value);
			}
			else
			{
				const UUID id = value.as<uint64_t>();
				size_t resourceIndex = propertyInfo->Offset();
				if (pMaterialData->ResourceCount() > resourceIndex) *pMaterialData->GetResourceUUIDPointer(manager, resourceIndex) = id;
			}
		}
	}

	void MaterialInstanceImporter::Initialize()
	{
	}

	void MaterialInstanceImporter::Cleanup()
	{
	}
}