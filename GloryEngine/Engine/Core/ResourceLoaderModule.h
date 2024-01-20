#pragma once
#include "Module.h"
#include "Resource.h"
#include "ResourceType.h"
#include "Engine.h"

#include <string>
#include <yaml-cpp/yaml.h>
#include <any>
#include <filesystem>

namespace Glory
{
	struct ImportSettings
	{
		ImportSettings();
		ImportSettings(const std::string& extension);

		std::string m_Extension;
	};

	/// <summary>
	/// Resource loaders only take care of loading resources, not managing them!
	/// </summary>
	class LoaderModule : public Module
	{
	public:
		LoaderModule();
		virtual ~LoaderModule();

		virtual Resource* LoadUsingAny(const std::string& path, const std::any& importSettings) = 0;
		virtual Resource* Load(const std::string& path, const ImportSettings& importSettings = ImportSettings()) = 0;
		virtual Resource* Load(const void* buffer, size_t length, const ImportSettings& importSettings = ImportSettings()) = 0;
		virtual void Save(const std::string& path, Resource* pResource) = 0;
		virtual const std::type_info& GetResourceType() = 0;

		virtual std::any ReadImportSettings(YAML::Node& node) = 0;
		virtual void WriteImportSettings(const std::any& importSettings, YAML::Emitter& out) = 0;

		virtual std::any CreateDefaultImportSettings(const std::string& extension) = 0;

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;

	private:
		friend class Engine;
		virtual bool HasPriority() override;
	};

	template<class T, typename S>
	class ResourceLoaderModule : public LoaderModule
	{
	public:
		ResourceLoaderModule(const std::string& extensions): m_Extensions(extensions)
		{
		}

		virtual ~ResourceLoaderModule() {}

		virtual Resource* LoadUsingAny(const std::string& path, const std::any& importSettings) override
		{
			std::filesystem::path filePath = path;
			std::string name = filePath.filename().replace_extension("").string();
			if (importSettings.type() != typeid(S))
			{
				std::any defaultSettings = CreateDefaultImportSettings(filePath.extension().string());
				S convertedSettings = std::any_cast<S>(defaultSettings);
				Resource* pResource = (Resource*)LoadResource(path, convertedSettings);
				if (pResource) pResource->SetName(name);
				return pResource;
			}

			S convertedSettings = std::any_cast<S>(importSettings);
			Resource* pResource = (Resource*)LoadResource(path, convertedSettings);
			if (pResource) pResource->SetName(name);
			return pResource;
		}

		virtual Resource* Load(const std::string& path, const ImportSettings& importSettings = ImportSettings()) override
		{
			std::filesystem::path filePath = path;
			std::string name = filePath.filename().replace_extension("").string();
			T* pResource = LoadResource(path, (const S&)importSettings);
			if (pResource) pResource->SetName(name);
			return (Resource*)pResource;
		}

		virtual Resource* Load(const void* buffer, size_t length, const ImportSettings& importSettings = ImportSettings()) override
		{
			T* pResource = LoadResource(buffer, length, (const S&)importSettings);
			return (Resource*)pResource;
		}

		virtual void Save(const std::string& path, Resource* pResource) override
		{
			SaveResource(path, (T*)pResource);
		}

		virtual const std::type_info& GetResourceType() override
		{
			return typeid(T);
		}

		virtual std::any ReadImportSettings(YAML::Node& node) override
		{
			return ReadImportSettings_Internal(node);
		}

		virtual void WriteImportSettings(const std::any& importSettings, YAML::Emitter& out) override
		{
			if (importSettings.type() != typeid(S))
			{
				std::any defaultSettings = CreateDefaultImportSettings("");
				S convertedSettings = std::any_cast<S>(defaultSettings);
				WriteImportSettings_Internal(convertedSettings, out);
				return;
			}

			S convertedSettings = std::any_cast<S>(importSettings);
			WriteImportSettings_Internal(convertedSettings, out);
		}

		virtual std::any CreateDefaultImportSettings(const std::string& extension) override
		{
			return CreateDefaultImportSettings_Internal(extension);
		}

	protected:
		virtual T* LoadResource(const std::string& path, const S& importSettings) = 0;
		virtual T* LoadResource(const void* buffer, size_t length, const S& importSettings) = 0;
		virtual void SaveResource(const std::string& path, T* pResource) {}
		virtual S ReadImportSettings_Internal(YAML::Node& node) = 0;
		virtual void WriteImportSettings_Internal(const S& importSettings, YAML::Emitter& out) = 0;

		virtual S CreateDefaultImportSettings_Internal(const std::string& extension)
		{
			return S(extension);
		}

	protected:
		virtual void Initialize()
		{
			m_pEngine->GetResourceTypes().RegisterResource<T>(m_Extensions);
		}
		virtual void Cleanup() = 0;

	private:
		std::string m_Extensions;
	};
}
