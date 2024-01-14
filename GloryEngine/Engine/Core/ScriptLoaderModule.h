#pragma once
#include "Script.h"
#include "ResourceLoaderModule.h"

namespace Glory
{
	struct ScriptImportSettings : public ImportSettings
	{
		ScriptImportSettings();
		ScriptImportSettings(const std::string& extension);
	};

	template<class T, typename S>
	class ScriptLoaderModule : public ResourceLoaderModule<T, S>
	{
	public:
		ScriptLoaderModule(const std::string& extensions) : ResourceLoaderModule<T, S>(extensions) {}
		virtual ~ScriptLoaderModule() {}

		virtual const std::type_info& GetModuleType() override { return typeid(ScriptLoaderModule); }

	protected:
		virtual S ReadImportSettings_Internal(YAML::Node& node) = 0;
		virtual void WriteImportSettings_Internal(const S& importSettings, YAML::Emitter& out) = 0;

	private:
		virtual T* LoadResource(const std::string& path, const S& importSettings) = 0;
		virtual T* LoadResource(const void* buffer, size_t length, const S& importSettings) = 0;
		virtual void SaveResource(const std::string& path, T* pResource) = 0;

	protected:
		virtual void Initialize() { ResourceLoaderModule::Initialize(); };
		virtual void Cleanup() {};
	};
}
