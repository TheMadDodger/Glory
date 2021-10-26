#include "ModelLoaderModule.h"

namespace Glory
{
	ModelLoaderModule::ModelLoaderModule() : ResourceLoaderModule("obj,fbx")
	{
	}

	ModelLoaderModule::~ModelLoaderModule()
	{
	}

	const std::type_info& ModelLoaderModule::GetModuleType()
	{
		return typeid(ModelLoaderModule);
	}

	ModelImportSettings ModelLoaderModule::ReadImportSettings_Internal(YAML::Node& node)
	{
		return ModelImportSettings();
	}

	void ModelLoaderModule::WriteImportSettings_Internal(const ModelImportSettings& importSettings, YAML::Emitter& out)
	{
	}

	ModelData* ModelLoaderModule::LoadResource(const std::string& path, const ModelImportSettings& importSettings)
	{
		ModelData* pModel = LoadModel(path, importSettings);
		return pModel;
	}

	ModelData* ModelLoaderModule::LoadResource(const void* buffer, size_t length, const ModelImportSettings& importSettings)
	{
		ModelData* pModel = LoadModel(buffer, length, importSettings);
		return pModel;
	}

	ModelImportSettings::ModelImportSettings() {}

	ModelImportSettings::ModelImportSettings(const std::string& extension)
		: ImportSettings(extension) {}
}
