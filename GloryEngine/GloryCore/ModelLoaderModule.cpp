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
		//pTexture->BuildTexture();
		return pModel;
	}
}
