#include "FSMEditor.h"
#include "FSMImporter.h"

#include <EditorApplication.h>
#include <EditorAssetDatabase.h>
#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <Undo.h>
#include <Shortcuts.h>

#include <FSM.h>
#include <AssetManager.h>

namespace Glory::Editor
{
	FSMEditor::FSMEditor(): m_EditingFSM(0), m_SelectedNode(0), m_EditingFSMIndex(0), m_ActiveNodeID(0), m_LastActiveNode(0)
	{
	}

	FSMEditor::~FSMEditor()
	{
	}

	void FSMEditor::SetFSM(UUID fsmID)
	{
		m_EditingFSM = fsmID;
		m_SelectedNode = 0;
		m_ActiveNodeID = 0;
		m_LastActiveNode = 0;
	}

	UUID FSMEditor::CurrentFSMID() const
	{
		return m_EditingFSM;
	}

	UUID& FSMEditor::SelectedNode()
	{
		return m_SelectedNode;
	}

	UUID& FSMEditor::ActiveNode()
	{
		return m_ActiveNodeID;
	}

	UUID& FSMEditor::LastActiveNode()
	{
		return m_LastActiveNode;
	}

	UUID& FSMEditor::DebuggingInstance()
	{
		return m_DebuggingInstance;
	}

	void FSMEditor::DeleteNode(UUID nodeID)
	{
		if (EditorApplication::GetInstance()->IsInPlayMode()) return;

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

	FSMData* FindFSM(const std::filesystem::path& path, Engine* pEngine)
	{
		const UUID fsmID = EditorAssetDatabase::FindAssetUUID(path.string());
		if (!fsmID) return nullptr;
		Resource* pResource = pEngine->GetAssetManager().FindResource(fsmID);
		if (!pResource) return nullptr;
		FSMData* pFSM = static_cast<FSMData*>(pResource);
		return pFSM;
	}

	void FSMEditor::Initialize()
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		Engine* pEngine = pApp->GetEngine();
		Serializers& serializers = pEngine->GetSerializers();
		EditorResourceManager& resources = pApp->GetResourceManager();

		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("Nodes"),
		[this, &serializers, pEngine](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			FSMData* pFSM = FindFSM(file.Path(), pEngine);
			if (!pFSM) return;

			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() == 2)
			{
				const std::string& idStr = components.back();
				auto node = file["Nodes"][idStr];
				if (!node.Exists() || !node.IsMap())
				{
					/* Node was deleted */
					const UUID nodeID = std::stoull(idStr);
					pFSM->RemoveNode(nodeID);
					return;
				}

				/* Node was added */
				const UUID nodeID = node["ID"].As<uint64_t>();
				const std::string name = node["Name"].As<std::string>();
				pFSM->NewNode(name, nodeID);
				return;
			}
			if (components.size() == 3 && components.back() == "Name")
			{
				/* Node was renamed */
				const std::string& idStr = components[1];
				auto node = file["Nodes"][idStr];
				const UUID nodeID = node["ID"].As<uint64_t>();
				const std::string name = node["Name"].As<std::string>();
				FSMNode* pNode = pFSM->FindNode(nodeID);
				if (pNode) pNode->m_Name = name;
				return;
			}
		});

		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("Transitions"),
		[this, &serializers, pEngine](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			FSMData* pFSM = FindFSM(file.Path(), pEngine);
			if (!pFSM) return;

			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() == 2)
			{
				const std::string& idStr = components.back();
				auto transition = file["Transitions"][idStr];
				if (!transition.Exists() || !transition.IsMap())
				{
					/* Transition was deleted */
					const UUID transitionID = std::stoull(idStr);
					pFSM->RemoveTransition(transitionID);
					return;
				}

				/* Transition was added */
				const UUID transitionID = transition["ID"].As<uint64_t>();
				const std::string name = transition["Name"].As<std::string>();
				const UUID from = transition["From"].As<uint64_t>();
				const UUID to = transition["To"].As<uint64_t>();
				const UUID prop = transition["Property"].As<uint64_t>();
				const FSMTransitionOP op = transition["OP"].AsEnum<FSMTransitionOP>();
				const float compareValue = transition["CompareValue"].As<float>();
				FSMTransition& newTransition = pFSM->NewTransition(name, from, to, transitionID);
				newTransition.m_Property = prop;
				newTransition.m_TransitionOp = op;
				newTransition.m_CompareValue = compareValue;
				return;
			}
			if (components.size() == 3)
			{
				/* Property changed */
				const std::string& idStr = components[1];
				auto transition = file["Transitions"][idStr];
				const UUID transitionID = transition["ID"].As<uint64_t>();
				const std::string name = transition["Name"].As<std::string>();
				const UUID from = transition["From"].As<uint64_t>();
				const UUID to = transition["To"].As<uint64_t>();
				const UUID prop = transition["Property"].As<uint64_t>();
				const FSMTransitionOP op = transition["OP"].AsEnum<FSMTransitionOP>();
				const float compareValue = transition["CompareValue"].As<float>();
				FSMTransition* pTransition = pFSM->FindTransition(transitionID);
				if (pTransition)
				{
					pTransition->m_Name = name;
					pTransition->m_FromNode = from;
					pTransition->m_ToNode = to;
					pTransition->m_Property = prop;
					pTransition->m_TransitionOp = op;
					pTransition->m_CompareValue = compareValue;
				}
				return;
			}
		});
		
		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("Properties"),
		[this, &serializers, pEngine](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			FSMData* pFSM = FindFSM(file.Path(), pEngine);
			if (!pFSM) return;

			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() == 2)
			{
				const std::string& idStr = components.back();
				auto prop = file["Properties"][idStr];
				if (!prop.Exists() || !prop.IsMap())
				{
					/* Property was deleted */
					const UUID propID = std::stoull(idStr);
					pFSM->RemoveProperty(propID);
					return;
				}

				/* Property was added */
				const UUID propID = prop["ID"].As<uint64_t>();
				const std::string name = prop["Name"].As<std::string>();
				const FSMPropertyType type = prop["Type"].AsEnum<FSMPropertyType>();
				pFSM->NewProperty(name, type, propID);
				return;
			}
			if (components.size() == 3)
			{
				/* Property was renamed or type was changed */
				const std::string& idStr = components[1];
				auto prop = file["Properties"][idStr];
				const UUID propID = prop["ID"].As<uint64_t>();
				const std::string name = prop["Name"].As<std::string>();
				const FSMPropertyType type = prop["Type"].AsEnum<FSMPropertyType>();
				FSMProperty* pProp = pFSM->FindProperty(propID);
				if (pProp)
				{
					pProp->m_Name = name;
					pProp->m_Type = type;
				}
				return;
			}
		});
		
		Undo::RegisterChangeHandler(std::string(".gfsm"), std::string("StartNode"),
		[this, &serializers, pEngine](Utils::YAMLFileRef& file, const std::filesystem::path& path) {
			FSMData* pFSM = FindFSM(file.Path(), pEngine);
			if (!pFSM) return;
				
			std::vector<std::string> components;
			Reflect::Tokenize(path.string(), components, '\\');
			if (components.size() != 1) return;
			const UUID startNode = file["StartNode"].As<uint64_t>();
			const size_t index = pFSM->NodeIndex(startNode);
			if (index == pFSM->NodeCount()) return;
			pFSM->SetStartNodeIndex(index);
		});

		Shortcuts::AddMainWindowAction("Delete", m_MainWindowIndex, [this, pEngine, &resources]() {
			if (!m_EditingFSM || !m_SelectedNode) return;
			DeleteNode(m_SelectedNode);
		});

		Shortcuts::AddMainWindowAction("Save Scene", m_MainWindowIndex, [this, pApp, &resources, pEngine]() {
			if (!m_EditingFSM) return;
			EditableResource* pResource = resources.GetEditableResource(m_EditingFSM);
			YAMLResource<FSMData>* pDocumentData = static_cast<YAMLResource<FSMData>*>(pResource);
			pDocumentData->Save();

			Resource* pFSMResource = pEngine->GetAssetManager().FindResource(m_EditingFSM);
			if (!pResource) return;
			FSMData* pFSM = static_cast<FSMData*>(pFSMResource);
			pFSM->Clear();
			FSMImporter::LoadInto(pFSM, **pDocumentData);
		});
	}

	void FSMEditor::OnUpdate()
	{
		if (!EditorApplication::GetInstance()->IsInPlayMode())
		{
			m_ActiveNodeID = 0;
			m_LastActiveNode = 0;
			m_DebuggingInstance = 0;
			return;
		}
	}
}
