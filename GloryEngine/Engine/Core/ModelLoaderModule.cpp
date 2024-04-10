#include "ModelLoaderModule.h"
#include "EngineProfiler.h"

namespace Glory
{
	ModelLoaderModule::ModelLoaderModule() : ResourceLoaderModule(".obj,.fbx,.gltf")
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
		ProfileSample s{ &m_pEngine->Profiler(), "ModelLoaderModule::LoadResource(path)" };
		ModelData* pModel = LoadModel(path, importSettings);
		return pModel;
	}

	ModelData* ModelLoaderModule::LoadResource(const void* buffer, size_t length, const ModelImportSettings& importSettings)
	{
		ProfileSample s{ &m_pEngine->Profiler(), "ModelLoaderModule::LoadResource(buffer)" };
		ModelData* pModel = LoadModel(buffer, length, importSettings);
		return pModel;
	}

	void ModelLoaderModule::Initialize()
	{
		ResourceLoaderModule::Initialize();
	}

	ModelImportSettings::ModelImportSettings() {}

	ModelImportSettings::ModelImportSettings(const std::string& extension)
		: ImportSettings(extension) {}
}
