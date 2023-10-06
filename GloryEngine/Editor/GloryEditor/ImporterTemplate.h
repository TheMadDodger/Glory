#pragma once
#include "Importer.h"

#include <ResourceType.h>
#include <string>
#include <Resource.h>

namespace Glory::Editor
{
	template<class T>
	class ImporterTemplate : public Importer
	{
	protected:
		ImporterTemplate() {}
		virtual ~ImporterTemplate() {}

		virtual Resource* Load(const std::filesystem::path& path) const override
		{
			const std::string name = path.filename().replace_extension("").string();
			T* pResource = LoadResource(path);
			if (pResource) pResource->SetName(name);
			return pResource;
		}

		virtual void Save(const std::filesystem::path& path, Resource* pResource) const override
		{
			SaveResource(path, (T*)pResource);
		}

		virtual uint32_t ResourceTypeHash() const override
		{
			return ResourceType::GetHash<T>();
		}

		virtual bool SupportsExtension(const std::filesystem::path& extension) const = 0;
		virtual T* LoadResource(const std::filesystem::path& path) const = 0;
		virtual void SaveResource(const std::filesystem::path& path, T* pResource) const {}

	protected:
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
	};
}
