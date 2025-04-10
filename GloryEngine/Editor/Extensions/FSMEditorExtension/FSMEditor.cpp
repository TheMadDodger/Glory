#include "FSMEditor.h"

#include <EditorApplication.h>
#include <EditorAssetDatabase.h>
#include <EditableResource.h>
#include <EditorResourceManager.h>
#include <Undo.h>

#include <FSM.h>

namespace Glory::Editor
{
	FSMEditor::FSMEditor(): m_EditingFSM(0), m_EditingFSMIndex(0)
	{
	}

	FSMEditor::~FSMEditor()
	{
	}

	void FSMEditor::SetFSM(UUID fsmID)
	{
		m_EditingFSM = fsmID;
	}

	UUID FSMEditor::CurrentFSMID() const
	{
		return m_EditingFSM;
	}

	std::string_view FSMEditor::Name()
	{
		return "";
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
	}

	void FSMEditor::OnUpdate()
	{
	}
}
