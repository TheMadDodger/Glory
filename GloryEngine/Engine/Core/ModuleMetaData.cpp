#include "ModuleMetaData.h"
#include "Debug.h"

#define READ(node, t, out) if (node.IsDefined())\
out = nameNode.as<t>()

#define READ_ARRAY(node, t, out) if (node.IsDefined() && node.IsSequence())\
for (size_t i = 0; i < node.size(); i++) \
{ \
YAML::Node nextNode = node[i]; \
if (nextNode.IsDefined()) out.push_back(nextNode.as<t>()); \
}\

#define YAML_READ(startNode, node, key, out, type) node = startNode[#key]; \
if (node.IsDefined()) out = node.as<type>()

#define YAML_READ_REQUIRED(startNode, node, key, out, type, error) node = startNode[#key]; \
if (node.IsDefined()) out = node.as<type>();\
else Debug::LogWarning(error)

namespace Glory
{
	std::map<std::string, ModuleType> ModuleMetaData::STRINGTOMODULETYPE =
	{
		{ "Window", ModuleType::MT_Window },
		{ "Renderer", ModuleType::MT_Renderer },
		{ "Graphics", ModuleType::MT_Graphics },
		{ "Loader", ModuleType::MT_Loader },
		{ "Scripting", ModuleType::MT_Scripting },
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
			Debug::LogWarning("Could not load Module.yaml at path: " + m_Path.string());
			return;
		}

		YAML::Node rootNode = YAML::LoadFile(m_Path.string());
		YAML::Node node;
		YAML_READ_REQUIRED(rootNode, node, Name, m_Name, std::string, "Missing Name property in Module.yaml at path: " + m_Path.string());
		std::string type = "";
		YAML_READ_REQUIRED(rootNode, node, Type, type, std::string, "Missing Type property in Module.yaml at path: " + m_Path.string());
		if (type == "SceneManagement")
		{
			Debug::LogError("Scene modules are no longer supported as of 0.3.0");
			m_Type = ModuleType::MT_Invalid;
			return;
		}
		if (type == "Physics")
		{
			Debug::LogWarning("As of 0.3.0 physics modules are now categorized as \"Other\" modules");
			type = "Other";
		}
		if (STRINGTOMODULETYPE.find(type) != STRINGTOMODULETYPE.end()) m_Type = STRINGTOMODULETYPE[type];

		YAML::Node dependenciesNode = rootNode["Dependencies"];
		READ_ARRAY(dependenciesNode, std::string, m_Dependencies);

		YAML::Node editorNode = rootNode["Editor"];
		if (!editorNode.IsDefined()) return;
		YAML_READ(editorNode, node, Backend, m_EditorBackend, std::string);
		YAML::Node extensionsNode = editorNode["Extensions"];
		READ_ARRAY(extensionsNode, std::string, m_EditorExtensions);

		ReadScriptingExtender(rootNode);
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

	const ModuleScriptingExtension* const ModuleMetaData::ScriptExtenderForLanguage(const std::string& language) const
	{
		if (m_ScriptingExtensions.find(language) == m_ScriptingExtensions.end()) return nullptr;
		return &m_ScriptingExtensions.at(language);
	}

	void ModuleMetaData::ReadScriptingExtender(YAML::Node& node)
	{
		YAML::Node scriptingNode = node["Scripting"];
		if (!scriptingNode.IsDefined() || !scriptingNode.IsSequence()) return;
		for (size_t i = 0; i < scriptingNode.size(); i++)
		{
			ModuleScriptingExtension scriptingExtension;
			YAML::Node subNode = scriptingNode[i];
			YAML::Node readNode;
			YAML_READ(subNode, readNode, Language, scriptingExtension.m_Language, std::string);
			YAML_READ(subNode, readNode, Extension, scriptingExtension.m_ExtensionFile, std::string);
			m_ScriptingExtensions.emplace(scriptingExtension.m_Language, scriptingExtension);
		}
	}
}
