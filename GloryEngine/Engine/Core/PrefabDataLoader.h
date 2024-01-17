#pragma once
#include "ResourceLoaderModule.h"
#include "PrefabData.h"

#include <NodeRef.h>

namespace Glory
{
	class PrefabDataLoader : public ResourceLoaderModule<PrefabData, ImportSettings>
	{
	public:
		PrefabDataLoader() : ResourceLoaderModule(".gentity") {}
		virtual ~PrefabDataLoader() = default;

		virtual const std::type_info& GetModuleType() override { return typeid(PrefabDataLoader); }

	protected:
		ImportSettings ReadImportSettings_Internal(YAML::Node& node) override { return {}; };
		void WriteImportSettings_Internal(const ImportSettings& importSettings, YAML::Emitter& out) override {}

	private:
		PrefabData* LoadResource(const std::string& path, const ImportSettings& importSettings);
		PrefabData* LoadResource(const void* buffer, size_t length, const ImportSettings& importSettings);
		void SaveResource(const std::string& path, PrefabData* pResource);

		void WriteChild(YAML::Emitter& out, const PrefabNode& parent);
		void ReadChild(PrefabData* pPrefab, Utils::NodeValueRef nodeValue, PrefabNode& node);

	protected:
		virtual void Initialize() { ResourceLoaderModule::Initialize(); };
		virtual void Cleanup() {};
	};
}
