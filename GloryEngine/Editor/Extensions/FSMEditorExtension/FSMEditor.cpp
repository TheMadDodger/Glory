#include "FSMEditor.h"

#include <EditorApplication.h>
#include <EditorAssetDatabase.h>
#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <Undo.h>
#include <Shortcuts.h>

#include <FSM.h>

namespace Glory::Editor
{
	FSMEditor::FSMEditor(): m_EditingFSM(0), m_SelectedNode(0), m_EditingFSMIndex(0)
	{
	}

	FSMEditor::~FSMEditor()
	{
	}

	void FSMEditor::SetFSM(UUID fsmID)
	{
		m_EditingFSM = fsmID;
		m_SelectedNode = 0;
	}

	UUID FSMEditor::CurrentFSMID() const
	{
		return m_EditingFSM;
	}

	UUID& FSMEditor::SelectedNode()
	{
		return m_SelectedNode;
	}

	void FSMEditor::DeleteNode(UUID nodeID)
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		EditorResourceManager& resources = EditorApplication::GetInstance()->GetResourceManager();
		EditableResource* pResource = resources.GetEditableResource(m_EditingFSM);
		YAMLResource<FSMData>* pDocument = static_cast<YAMLResource<FSMData>*>(pResource);
		Utils::YAMLFileRef& file = **pDocument;

		auto entryNode = file["StartNode"];
		auto nodes = file["Nodes"];
		auto transitions = file["Transitions"];

		const UUID entryNodeID = entryNode.As<uint64_t>();

		std::string idStr = std::to_string(nodeID);
		auto node = nodes[idStr];

		Undo::StartRecord("Remove Node");
		Undo::YAMLEdit(file, node.Path(), node.Node(), YAML::Node{ YAML::NodeType::Null });

		std::vector<std::string> toRemoveTransitions;
		for (auto iter = transitions.Begin(); iter != transitions.End(); ++iter)
		{
			const std::string key = *iter;
			auto transition = transitions[key];
			auto from = transition["From"];
			auto to = transition["To"];

			const UUID startNodeID = from.As<uint64_t>();
			const UUID endNodeID = to.As<uint64_t>();

			if (startNodeID != nodeID && endNodeID != nodeID) continue;
			toRemoveTransitions.push_back(key);
		}

		for (size_t i = 0; i < toRemoveTransitions.size(); ++i)
		{
			const std::string& key = toRemoveTransitions[i];
			auto transition = transitions[key];
			Undo::YAMLEdit(file, transition.Path(), transition.Node(), YAML::Node{ YAML::NodeType::Null });
		}

		if (entryNodeID == nodeID)
		{
			UUID newEntryNode = 0;
			if (nodes.Begin() != nodes.End())
			{
				auto node = nodes[*nodes.Begin()];
				newEntryNode = node["ID"].As<uint64_t>();
			}
			Undo::ApplyYAMLEdit(file, entryNode.Path(), uint64_t(entryNodeID), uint64_t(newEntryNode));
		}

		Undo::StopRecord();

		if (m_SelectedNode == nodeID) m_SelectedNode = 0;
	}

	std::string_view FSMEditor::Name()
	{
		return "FSM";
	}

	void FSMEditor::OnGui(float height)
	{
		Dockspace(height);
	}

	void FSMEditor::Initialize()
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		Engine* pEngine = pApp->GetEngine();
		Serializers& serializers = pEngine->GetSerializers();
		EditorResourceManager& resources = pApp->GetResourceManager();

		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("Nodes"),
		[this, &serializers](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() <= 5) return;

		});

		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("Transitions"),
		[this, &serializers](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() <= 5) return;

		});
		
		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("Properties"),
		[this, &serializers](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() <= 5) return;

		});
		
		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("StartNode"),
		[this, &serializers](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() <= 5) return;

		});

		Shortcuts::AddMainWindowAction("Delete", m_MainWindowIndex, [this, pEngine, &resources]() {
			if (!m_EditingFSM || !m_SelectedNode) return;
			DeleteNode(m_SelectedNode);
		});

		Shortcuts::AddMainWindowAction("Save Scene", m_MainWindowIndex, [this, pApp, &resources]() {
			if (!m_EditingFSM) return;
			EditableResource* pResource = resources.GetEditableResource(m_EditingFSM);
			YAMLResource<FSMData>* pDocumentData = static_cast<YAMLResource<FSMData>*>(pResource);
			pDocumentData->Save();
		});
	}

	void FSMEditor::OnUpdate()
	{
	}
}
