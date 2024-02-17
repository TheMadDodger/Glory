#pragma once
#include "Importer.h"

#include <ResourceType.h>
#include <string>
#include <Resource.h>
#include <EditableResource.h>

namespace Glory::Editor
{
	template<class T>
	class ImporterTemplate : public Importer
	{
	protected:
		ImporterTemplate() {}
		virtual ~ImporterTemplate() {}

		virtual ImportedResource Load(const std::filesystem::path& path, void* userData) const override
		{
			const std::string name = path.filename().replace_extension("").string();
			ImportedResource pResource = LoadResource(path, userData);
			if (pResource) pResource->SetName(name);
			return pResource;
		}

		virtual bool Save(const std::filesystem::path& path, Resource* pResource) const override
		{
			return SaveResource(path, (T*)pResource);
		}

		virtual uint32_t ResourceTypeHash() const override
		{
			return ResourceTypes::GetHash<T>();
		}

		virtual EditableResource* GetEditableResource(const std::filesystem::path&) const override
		{
			return new NonEditableResource<T>();
		}

		virtual bool SupportsExtension(const std::filesystem::path& extension) const = 0;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void* userData) const = 0;
		virtual bool SaveResource(const std::filesystem::path& path, T* pResource) const { return false; }

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
	};

	template<class T>
	class YAMLImporterTemplate : public ImporterTemplate<T>
	{
	protected:
		virtual EditableResource* GetEditableResource(const std::filesystem::path& path) const override
		{
			return new YAMLResource<T>(path);
		}
	};
}
