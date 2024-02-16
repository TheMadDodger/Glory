#pragma once
#include "GloryEditor.h"

#include <vector>
#include <map>
#include <string>

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
		GLORY_EDITOR_API ImportedResource(Resource* pResource);
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

	private:
		friend class EditorAssetDatabase;
		Resource* m_pResource;
		std::vector<ImportedResource> m_Children;
		std::map<std::string, size_t> m_NameToSubresourceIndex;
	};
}
}
