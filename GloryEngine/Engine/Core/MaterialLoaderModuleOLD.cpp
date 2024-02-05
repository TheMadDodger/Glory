//namespace Glory
//{
//	void MaterialLoaderModule::SaveResource(const std::string& path, MaterialData* pResource)
//	{
//		MaterialInstanceData* pMaterialInstance = dynamic_cast<MaterialInstanceData*>(pResource);
//		YAML::Emitter out;
//		SaveMaterialData(pResource, out);
//		std::ofstream outStream(path);
//		outStream << out.c_str();
//		outStream.close();
//	}
//
//	void MaterialLoaderModule::SaveMaterialData(MaterialData* pMaterialData, YAML::Emitter& out)
//	{
//		out << YAML::BeginMap;
//		WriteShaders(out, pMaterialData);
//		WritePropertyData(out, pMaterialData);
//		out << YAML::EndMap;
//	}
//
//	void MaterialLoaderModule::WriteShaders(YAML::Emitter& out, MaterialData* pMaterialData)
//	{
//		out << YAML::Key << "Shaders";
//		out << YAML::Value << YAML::BeginSeq;
//		for (size_t i = 0; i < pMaterialData->ShaderCount(); i++)
//		{
//			out << YAML::BeginMap;
//			ShaderSourceData* pShaderSourceData = pMaterialData->GetShaderAt(i);
//			YAML_WRITE(out, UUID, pShaderSourceData->GetUUID());
//			YAML_WRITE(out, Type, pShaderSourceData->GetShaderType());
//			out << YAML::EndMap;
//		}
//		out << YAML::EndSeq;
//	}
//
//	void MaterialLoaderModule::WritePropertyData(YAML::Emitter& out, MaterialData* pMaterialData)
//	{
//		out << YAML::Key << "Properties";
//		out << YAML::Value << YAML::BeginSeq;
//
//		size_t resourceIndex = 0;
//		for (size_t i = 0; i < pMaterialData->PropertyInfoCount(); i++)
//		{
//			out << YAML::BeginMap;
//
//			MaterialPropertyInfo* pPropertyInfo = pMaterialData->GetPropertyInfoAt(i);
//			std::string name;
//			YAML_WRITE(out, DisplayName, pPropertyInfo->DisplayName());
//			YAML_WRITE(out, ShaderName, pPropertyInfo->ShaderName());
//			YAML_WRITE(out, TypeHash, pPropertyInfo->TypeHash());
//
//			bool isResource = m_pEngine->GetResourceTypes().IsResource(pPropertyInfo->m_TypeHash);
//			if (!isResource)
//			{
//				m_pEngine->GetSerializers().SerializeProperty("Value", pMaterialData->GetBufferReference(), pPropertyInfo->m_TypeHash, pPropertyInfo->m_Offset, pPropertyInfo->m_Size, out);
//			}
//			else
//			{
//				size_t index = pMaterialData->GetPropertyIndexFromResourceIndex(resourceIndex);
//				++resourceIndex;
//				const UUID uuid = pMaterialData->GetResourceUUIDPointer(index)->AssetUUID();
//				out << YAML::Key << "Value" << YAML::Value << uuid;
//			}
//
//			out << YAML::EndMap;
//		}
//
//		out << YAML::EndSeq;
//	}
//}
