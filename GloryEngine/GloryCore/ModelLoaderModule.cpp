#include "ModelLoaderModule.h"

namespace Glory
{
	ModelLoaderModule::ModelLoaderModule()
	{
	}

	ModelLoaderModule::~ModelLoaderModule()
	{
	}

	const std::type_info& ModelLoaderModule::GetModuleType()
	{
		return typeid(ModelLoaderModule);
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
}
