#include "ImportedResource.h"
#include "EditorAssetDatabase.h"

#include <Debug.h>
#include <Resource.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
	ImportedResource::ImportedResource(): m_Path(""), m_pResource(nullptr), m_Claimed(false)
	{
	}

	ImportedResource::ImportedResource(std::nullptr_t): m_Path(""), m_pResource(nullptr), m_Claimed(false)
	{
	}

	ImportedResource::ImportedResource(const std::filesystem::path& path, Resource* pResource):
		m_Path(path), m_pResource(pResource), m_Claimed(false)
	{
		const auto reserve = EditorAssetDatabase::ReserveAssetUUID(m_Path.string(), "");
		m_IsNew = !reserve.second;
		if (reserve.first) pResource->SetResourceUUID(reserve.first);
	}

	ImportedResource& ImportedResource::AddChild(Resource* pResource, const std::string& name)
	{
		const size_t index = m_Children.size();
		if (m_NameToSubresourceIndex.find(name) != m_NameToSubresourceIndex.end())
			EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("Could not add Subresource because a Subresource with the same name already exists!");
		else
			m_NameToSubresourceIndex.emplace(name, index);

		pResource->SetName(name);
		m_Children.push_back({});
		ImportedResource& resource = m_Children[index];
		resource.m_Path = m_Path;
		resource.m_pResource = pResource;

		resource.m_CachedSubPath = m_CachedSubPath;
		resource.m_CachedSubPath.append(name);

		const auto reserve = EditorAssetDatabase::ReserveAssetUUID(m_Path.string(), resource.m_CachedSubPath);
		resource.m_IsNew = !reserve.second;
		if (reserve.first) pResource->SetResourceUUID(reserve.first);
		return resource;
	}

	ImportedResource& ImportedResource::AddChild(ImportedResource&& child, const std::string& name)
	{
		const size_t index = m_Children.size();

		if (m_NameToSubresourceIndex.find(name) == m_NameToSubresourceIndex.end())
		{
			//m_pEngine->GetDebug().LogError("Could not add Subresource because a Subresource with the same name already exists!");
			m_NameToSubresourceIndex.emplace(name, index);
		}
		
		//child.m_pParent = this;

		m_Children.push_back(std::move(child));
		return m_Children[index];
	}

	Resource* ImportedResource::operator->()
	{
		return m_pResource;
	}

	const Resource* ImportedResource::operator->() const
	{
		return m_pResource;
	}

	Resource* ImportedResource::operator*()
	{
		m_Claimed = true;
		return m_pResource;
	}

	ImportedResource::operator bool() const
	{
		return m_pResource;
	}

	size_t ImportedResource::ChildCount() const
	{
		return m_Children.size();
	}

	ImportedResource& ImportedResource::Child(size_t index)
	{
		return m_Children[index];
	}

	const ImportedResource& ImportedResource::Child(size_t index) const
	{
		return m_Children[index];
	}

	ImportedResource* ImportedResource::Child(const std::string& name)
	{
		auto itor = m_NameToSubresourceIndex.find(name);
		if (itor == m_NameToSubresourceIndex.end()) return nullptr;
		return &m_Children[itor->second];
	}

	const ImportedResource* ImportedResource::Child(const std::string& name) const
	{
		auto itor = m_NameToSubresourceIndex.find(name);
		if (itor == m_NameToSubresourceIndex.end()) return nullptr;
		return &m_Children[itor->second];
	}

	ImportedResource* ImportedResource::ChildFromPath(const std::filesystem::path& path)
	{
		auto subPathItor = path.begin();
		if (subPathItor == path.end()) return this;
		std::filesystem::path subPath = *subPathItor;
		if (subPath == ".") return this;
		ImportedResource* pSubresource = Child(subPath.string());
		if (!pSubresource) return nullptr;
		std::filesystem::path nextPath = path.lexically_relative(subPath);
		return pSubresource->ChildFromPath(nextPath);
	}

	const ImportedResource* ImportedResource::ChildFromPath(const std::filesystem::path& path) const
	{
		auto subPathItor = path.begin();
		if (subPathItor == path.end()) return this;
		std::filesystem::path subPath = *subPathItor;
		if (subPath == ".") return this;
		const ImportedResource* pSubresource = Child(subPath.string());
		if (!pSubresource) return nullptr;
		std::filesystem::path nextPath = path.lexically_relative(subPath);
		return pSubresource->ChildFromPath(nextPath);
	}

	bool ImportedResource::IsNew() const
	{
		return m_IsNew;
	}

	const std::filesystem::path& ImportedResource::Path() const
	{
		return m_Path;
	}

	const std::filesystem::path& ImportedResource::SubPath() const
	{
		return m_CachedSubPath;
	}

	void ImportedResource::Cleanup()
	{
		if (m_pResource && !m_Claimed)
		{
			delete m_pResource;
			m_pResource = nullptr;
		}

		for (size_t i = 0; i < m_Children.size(); ++i)
		{
			m_Children[i].Cleanup();
		}
	}
}
