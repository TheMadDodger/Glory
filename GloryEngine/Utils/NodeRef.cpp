#include "NodeRef.h"
#include <fstream>

namespace Glory::Utils
{
	NodeRef::NodeRef(YAML::Node& node)
		: m_RootNode(node), m_RootValueRef(node, "") {}

	NodeValueRef NodeRef::operator[](const std::filesystem::path& path)
	{
		return m_RootValueRef[path];
	}

	NodeValueRef NodeRef::ValueRef()
	{
		return m_RootValueRef;
	}

	NodeValueRef::NodeValueRef(YAML::Node& rootNode, const std::filesystem::path& path)
		: m_RootNode(rootNode), m_Path(path)
	{
	}

	NodeValueRef NodeValueRef::operator[](const std::filesystem::path& path)
	{
		std::filesystem::path fsPath = m_Path;
		fsPath.append(path.string());
		return NodeValueRef(m_RootNode, fsPath.string());
	}

	NodeValueRef NodeValueRef::operator[](const size_t index)
	{
		std::filesystem::path fsPath = m_Path;
		fsPath.append("##" + std::to_string(index));
		return NodeValueRef(m_RootNode, fsPath);
	}

	void NodeValueRef::Set(YAML::Node& node)
	{
		Node() = node;
	}

	void NodeValueRef::PushBack(YAML::Node& node)
	{
		Node().push_back(node);
	}

	void NodeValueRef::Remove(size_t index)
	{
		Node().remove(index);
	}

	void NodeValueRef::Remove(const std::string& key)
	{
		Node().remove(key);
	}

	void NodeValueRef::Insert(YAML::Node& node, size_t index)
	{
		Node().force_insert(index, node);
	}

	size_t NodeValueRef::Size()
	{
		if (!Exists()) return 0;
		return Node().size();
	}

	bool NodeValueRef::Exists() const
	{
		return Node().IsDefined();
	}

	bool NodeValueRef::IsScalar() const
	{
		return Node().IsScalar();
	}

	bool NodeValueRef::IsSequence() const
	{
		return Node().IsSequence();
	}

	bool NodeValueRef::IsMap() const
	{
		return Node().IsMap();
	}

	const std::filesystem::path& NodeValueRef::Path()
	{
		return m_Path;
	}

	void NodeValueRef::Erase()
	{
		NodeValueRef parent = Parent();
		const std::filesystem::path& path = m_Path.lexically_relative(parent.Path());
		const std::string& pathString = path.string();
		if (pathString._Starts_with("##"))
		{
			const size_t index = std::stoul(pathString.substr(2));
			parent.Remove(index);
			return;
		}
		parent.Remove(path.string());
	}

	NodeValueRef NodeValueRef::Parent()
	{
		return NodeValueRef(m_RootNode, m_Path.parent_path());
	}

	NodeValueRef::Iterator NodeValueRef::Begin() const
	{
		YAML::const_iterator iter = Node().begin();
		return NodeValueRef::Iterator{ iter };
	}

	NodeValueRef::Iterator NodeValueRef::End() const
	{
		YAML::const_iterator iter = Node().end();
		return NodeValueRef::Iterator{ iter };
	}

	YAML::Node NodeValueRef::FindNode(YAML::Node& node, std::filesystem::path path)
	{
		if (path.empty() || path == ".") return node;

		const std::string& subPathString = (*path.begin()).string();
		if (subPathString._Starts_with("##"))
		{
			const size_t index = std::stoul(subPathString.substr(2));
			path = path.lexically_relative(subPathString);
			YAML::Node& nextNode = node[index];
			return FindNode(nextNode, path);
		}

		YAML::Node& nextNode = node[subPathString];
		path = path.lexically_relative(subPathString);
		return FindNode(nextNode, path);
	}

	const YAML::Node NodeValueRef::FindNode(YAML::Node& node, std::filesystem::path path) const
	{
		if (path.empty() || path == ".") return node;

		const std::string& subPathString = (*path.begin()).string();
		if (subPathString._Starts_with("##"))
		{
			const size_t index = std::stoul(subPathString.substr(2));
			path = path.lexically_relative(subPathString);
			YAML::Node& nextNode = node[index];
			return FindNode(nextNode, path);
		}

		YAML::Node& nextNode = node[subPathString];
		path = path.lexically_relative(subPathString);
		return FindNode(nextNode, path);
	}

	YAML::Node NodeValueRef::Node()
	{
		if (m_Path.empty() || m_Path == ".") return m_RootNode;
		return FindNode(m_RootNode, m_Path);
	}

	const YAML::Node NodeValueRef::Node() const
	{
		if (m_Path.empty() || m_Path == ".") return m_RootNode;
		return FindNode(m_RootNode, m_Path);
	}

	YAMLFileRef::YAMLFileRef()
		: m_FilePath(), m_RootNode()
	{
	}

	YAMLFileRef::YAMLFileRef(const std::filesystem::path& filePath)
		: m_FilePath(filePath)
	{
		if(std::filesystem::exists(filePath))
			m_RootNode = YAML::LoadFile(filePath.string());
	}

	void YAMLFileRef::Load()
	{
		m_RootNode = YAML::LoadFile(m_FilePath.string());
	}

	void YAMLFileRef::Save()
	{
		YAML::Emitter out;
		out << m_RootNode;
		std::ofstream file(m_FilePath);
		file << out.c_str();
		file.close();
	}

	const std::filesystem::path& YAMLFileRef::Path() const
	{
		return m_FilePath;
	}

	void YAMLFileRef::ChangePath(const std::filesystem::path& newPath)
	{
		m_FilePath = newPath;
	}

	NodeRef YAMLFileRef::RootNodeRef()
	{
		return NodeRef(m_RootNode);
	}

	NodeValueRef YAMLFileRef::operator[](const std::filesystem::path& path)
	{
		return RootNodeRef()[path];
	}
}
