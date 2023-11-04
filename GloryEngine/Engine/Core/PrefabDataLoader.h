#pragma once
#include "ResourceLoaderModule.h"

namespace Glory
{
	template<class T, typename S>
	class PrefabDataLoader : public ResourceLoaderModule<T, S>
	{
	public:
		PrefabDataLoader(const std::string& extensions) : ResourceLoaderModule<T, S>(extensions) {}
		virtual ~PrefabDataLoader() {}

		virtual const std::type_info& GetModuleType() override { return typeid(PrefabDataLoader); }

	protected:
		virtual S ReadImportSettings_Internal(YAML::Node& node) = 0;
		virtual void WriteImportSettings_Internal(const S& importSettings, YAML::Emitter& out) = 0;

	private:
		virtual T* LoadResource(const std::string& path, const S& importSettings) = 0;
		virtual T* LoadResource(const void* buffer, size_t length, const S& importSettings) = 0;
		virtual void SaveResource(const std::string& path, T* pResource) = 0;

	protected:
		virtual void Initialize() {};
		virtual void Cleanup() {};
	};
}
