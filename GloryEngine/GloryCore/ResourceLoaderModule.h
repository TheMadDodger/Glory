#pragma once
#include "Module.h"
#include "Resource.h"
#include <string>

namespace Glory
{
	struct ImportSettings {};

	/// <summary>
	/// Resource loaders only take care of loading resources, not managing them!
	/// </summary>
	class LoaderModule : public Module
	{
	public:
		LoaderModule();
		virtual ~LoaderModule();

		virtual Resource* Load(const std::string& path, const ImportSettings& importSettings = ImportSettings()) = 0;
		virtual const std::type_info& GetResourceType() = 0;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void Update() = 0;
		virtual void Draw() = 0;
	};

	template<class T, typename S>
	class ResourceLoaderModule : public LoaderModule
	{
	public:
		ResourceLoaderModule() {}
		virtual ~ResourceLoaderModule() {}

		virtual Resource* Load(const std::string& path, const ImportSettings& importSettings = ImportSettings()) override
		{
			T* pResource = LoadResource(path, (const S&)importSettings);
			return (Resource*)pResource;
		}

		virtual const std::type_info& GetResourceType() override
		{
			return typeid(T);
		}

	protected:
		virtual T* LoadResource(const std::string& path, const S& importSettings) = 0;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void Update() = 0;
		virtual void Draw() = 0;
	};
}
