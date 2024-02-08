#include "MaterialInstanceImporter.h"
#include "EditorApplication.h"

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
		YAML::Node rootNode = YAML::LoadFile(path.string());
		return LoadMaterialInstanceData(rootNode);
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

	MaterialInstanceData* MaterialInstanceImporter::LoadMaterialInstanceData(YAML::Node& rootNode) const
	{
		YAML::Node node;
		UUID baseMaterial = 0;
		YAML_READ(rootNode, node, BaseMaterial, baseMaterial, uint64_t);
		MaterialInstanceData* pMaterialInstanceData = new MaterialInstanceData(baseMaterial);
		ReadPropertyOverrides(rootNode, pMaterialInstanceData);
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

	void MaterialInstanceImporter::ReadPropertyOverrides(YAML::Node& rootNode, MaterialInstanceData* pMaterialData) const
	{
		MaterialManager& manager = EditorApplication::GetInstance()->GetEngine()->GetMaterialManager();

		YAML::Node propertiesNode = rootNode["Overrides"];
		if (!propertiesNode.IsSequence()) return;

		for (size_t i = 0; i < propertiesNode.size(); i++)
		{
			YAML::Node propertyNode = propertiesNode[i];
			YAML::Node node;
			std::string displayName;
			YAML_READ(propertyNode, node, DisplayName, displayName, std::string);

			size_t propertyIndex = 0;
			if (!pMaterialData->GetPropertyInfoIndex(manager, displayName, propertyIndex)) continue;
			pMaterialData->EnableProperty(propertyIndex);

			MaterialPropertyInfo* propertyInfo = pMaterialData->GetPropertyInfoAt(manager, propertyIndex);

			node = propertyNode["Value"];

			if (!propertyInfo->IsResource())
			{
				uint32_t typeHash = propertyInfo->TypeHash();
				size_t offset = propertyInfo->Offset();
				size_t size = propertyInfo->Size();
				EditorApplication::GetInstance()->GetEngine()->GetSerializers().DeserializeProperty(pMaterialData->GetBufferReference(manager), typeHash, offset, size, node);
			}
			else
			{
				UUID id = node.as<uint64_t>();
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