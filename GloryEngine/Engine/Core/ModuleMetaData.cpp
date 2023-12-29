#include "ModuleMetaData.h"
#include "Debug.h"

#include <NodeRef.h>

#include <map>

#define YAML_READ(startNode, node, key, out, type) node = startNode[#key]; \
if (node.IsDefined()) out = node.as<type>()

#define YAML_READ_REQUIRED(startNode, node, key, out, type, error) node = startNode[#key]; \
if (node.IsDefined()) out = node.as<type>();\
else m_pEngine->GetDebug().LogWarning(error)

#define YAML_READ_REQUIRED_1(startNode, node, key, out, type, error) node = startNode[#key]; \
if (node.IsDefined()) out = node.as<type>();

namespace Glory
{
	std::map<std::string, ModuleType> STRINGTOMODULETYPE =
	{
		{ "Window", ModuleType::MT_Window },
		{ "Renderer", ModuleType::MT_Renderer },
		{ "Graphics", ModuleType::MT_Graphics },
		{ "Loader", ModuleType::MT_Loader },
		{ "Input", ModuleType::MT_Input },
		{ "Other", ModuleType::MT_Other },
	};

	ModuleMetaData::ModuleMetaData()
		: m_Path(""), m_Name(""), m_Type(ModuleType::MT_Other), m_EditorBackend(""), m_EditorExtensions()
	{
	}

	ModuleMetaData::ModuleMetaData(const std::filesystem::path& path)
		: m_Path(path), m_Name(""), m_Type(ModuleType::MT_Other), m_EditorBackend(""), m_EditorExtensions()
	{
	}

	ModuleMetaData::~ModuleMetaData()
	{
	}

	void ModuleMetaData::Read()
	{
		if (!std::filesystem::exists(m_Path))
		{
			//m_pEngine->GetDebug().LogWarning("Could not load Module.yaml at path: " + m_Path.string());
			return;
		}

		Utils::YAMLFileRef metaFile{ m_Path };
		metaFile.Load();

		Utils::NodeRef rootNode = metaFile.RootNodeRef();

		m_Name = rootNode["Name"].As<std::string>();
		std::string type = rootNode["Type"].As<std::string>();

		if (type == "SceneManagement")
		{
			//m_pEngine->GetDebug().LogError("Scene modules are no longer supported as of 0.3.0");
			m_Type = ModuleType::MT_Invalid;
			return;
		}
		if (type == "Physics")
		{
			//m_pEngine->GetDebug().LogWarning("As of 0.3.0 physics modules are now categorized as \"Other\" modules");
			type = "Other";
		}
		if (type == "Scripting")
		{
			Debug::LogWarning("As of 0.3.0 scripting modules are now categorized as \"Other\" modules");
			type = "Other";
		}
		if (STRINGTOMODULETYPE.find(type) != STRINGTOMODULETYPE.end()) m_Type = STRINGTOMODULETYPE[type];

		Utils::NodeValueRef dependenciesNode = rootNode["Dependencies"];
		for (size_t i = 0; i < dependenciesNode.Size(); ++i) \
		{
			Utils::NodeValueRef nextNode = dependenciesNode[i]; \
			m_Dependencies.push_back(nextNode.As<std::string>()); \
		}

		Utils::NodeValueRef editorNode = rootNode["Editor"];
		if (editorNode.Exists())
		{
			m_EditorBackend = editorNode["Backend"].Exists() ? editorNode["Backend"].As<std::string>() : "";
			Utils::NodeValueRef extensionsNode = editorNode["Extensions"];
			for (size_t i = 0; i < extensionsNode.Size(); ++i) \
			{
				Utils::NodeValueRef nextNode = extensionsNode[i]; \
				m_EditorExtensions.push_back(nextNode.As<std::string>()); \
			}
		}

		/* Read extras */
		Utils::NodeValueRef extrasNode = rootNode["Extras"];
		if (!extrasNode.Exists() || !extrasNode.IsSequence()) return;
		for (size_t i = 0; i < extrasNode.Size(); i++)
		{
			ModuleExtra extra;
			Utils::NodeValueRef subNode = extrasNode[i];
			extra.m_File = subNode["File"].As<std::string>();
			extra.m_Requires = subNode["Requires"].As<std::string>();
			m_ModuleExtras.push_back(extra);
		}
	}

	const std::filesystem::path& ModuleMetaData::Path() const
	{
		return m_Path;
	}

	const std::string& ModuleMetaData::Name() const
	{
		return m_Name;
	}

	const ModuleType& ModuleMetaData::Type() const
	{
		return m_Type;
	}

	const std::string& ModuleMetaData::EditorBackend() const
	{
		return m_EditorBackend;
	}

	const std::vector<std::string>& ModuleMetaData::EditorExtensions() const
	{
		return m_EditorExtensions;
	}

	const std::vector<std::string>& ModuleMetaData::Dependencies() const
	{
		return m_Dependencies;
	}

	const size_t ModuleMetaData::NumExtras() const
	{
		return m_ModuleExtras.size();
	}

	const ModuleExtra& ModuleMetaData::Extra(size_t index) const
	{
		return m_ModuleExtras.at(index);
	}
}
