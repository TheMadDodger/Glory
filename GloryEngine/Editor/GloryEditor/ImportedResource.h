#pragma once
#include "GloryEditor.h"

#include <vector>
#include <map>
#include <string>
#include <filesystem>

namespace std::filesystem
{
	class path;
}

namespace Glory
{
	class Resource;

namespace Editor
{
	struct ImportedResource
	{
	public:
		GLORY_EDITOR_API ImportedResource();
		GLORY_EDITOR_API ImportedResource(std::nullptr_t);
		GLORY_EDITOR_API ImportedResource(const std::filesystem::path& path, Resource* pResource);
		GLORY_EDITOR_API ~ImportedResource() = default;

		GLORY_EDITOR_API ImportedResource& AddChild(Resource* pResource, const std::string& name);
		GLORY_EDITOR_API ImportedResource& AddChild(ImportedResource&& child, const std::string& name);

		GLORY_EDITOR_API Resource* operator->();
		GLORY_EDITOR_API const Resource* operator->() const;
		GLORY_EDITOR_API Resource* operator*();
		GLORY_EDITOR_API operator bool() const;

		GLORY_EDITOR_API size_t ChildCount() const;
		GLORY_EDITOR_API ImportedResource& Child(size_t index);
		GLORY_EDITOR_API const ImportedResource& Child(size_t index) const;
		GLORY_EDITOR_API ImportedResource* Child(const std::string& name);
		GLORY_EDITOR_API const ImportedResource* Child(const std::string& name) const;

		GLORY_EDITOR_API ImportedResource* ChildFromPath(const std::filesystem::path& path);
		GLORY_EDITOR_API const ImportedResource* ChildFromPath(const std::filesystem::path& path) const;
		GLORY_EDITOR_API bool IsNew() const;
		GLORY_EDITOR_API const std::filesystem::path& Path() const;
		GLORY_EDITOR_API const std::filesystem::path& SubPath() const;

		GLORY_EDITOR_API void Cleanup();

	private:
		friend class EditorAssetDatabase;
		friend class Importer;
		Resource* m_pResource;
		std::vector<ImportedResource> m_Children;
		std::map<std::string, size_t> m_NameToSubresourceIndex;
		bool m_IsNew = false;
		std::filesystem::path m_Path;
		std::filesystem::path m_CachedSubPath;
	};
}
}
