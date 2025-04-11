#include "FSMEditorExtension.h"
#include "FSMImporter.h"
#include "FSMTumbnailGenerator.h"
#include "FSMEditor.h"
#include "FSMPropertiesWindow.h"
#include "FSMNodeEditor.h"
#include "FSMNodeInspector.h"

#include <FSM.h>
#include <FSMModule.h>

#include <ObjectMenu.h>
#include <FileBrowser.h>
#include <EditorAssetDatabase.h>
#include <ObjectMenuCallbacks.h>
#include <Tumbnail.h>
#include <MenuBar.h>

#include <IconsFontAwesome6.h>
#include <PropertyDrawer.h>

EXTENSION_CPP(FSMEditorExtension)

namespace Glory::Editor
{
	static constexpr char* Shortcut_Window_FSMProperties = "Open FSM Properties";
	static constexpr char* Shortcut_Window_FSMNodes = "Open FSM Node Editor";
	static constexpr char* Shortcut_Window_FSMInspector = "Open FSM Inspector";

	FSMImporter Importer;
	FSMEditor Editor;

	FSMEditorExtension::FSMEditorExtension()
	{
	}

	FSMEditorExtension::~FSMEditorExtension()
	{
	}

	void OnCreateFSM(Object*, const ObjectMenuType&)
	{
		std::filesystem::path path = FileBrowserItem::GetCurrentPath();
		path = GetUnqiueFilePath(path.append("New State Machine.gfsm"));

		FileBrowser::BeginCreate(path.filename().replace_extension("").string(), "", [](std::filesystem::path& finalPath) {
			finalPath.replace_extension("gfsm");
			if (std::filesystem::exists(finalPath)) return;
			FSMData* pFSM = new FSMData();

			UUID node1;
			UUID node2;
			UUID node3;
			UUID node4;
			UUID trigger1;
			UUID trigger2;
			UUID trigger3;
			UUID trigger4;
			UUID trigger5;

			pFSM->NewNode("State 1", node1);
			pFSM->NewNode("State 2", node2);
			pFSM->NewNode("State 3", node3);
			pFSM->NewNode("State 4", node4);

			pFSM->NewTransition("Transition 1", node1, node2).m_Property = trigger1;
			pFSM->NewTransition("Transition 2", node1, node3).m_Property = trigger2;
			pFSM->NewTransition("Transition 3", node2, node3).m_Property = trigger3;
			pFSM->NewTransition("Transition 4", node3, node4).m_Property = trigger4;
			pFSM->NewTransition("Transition 5", node4, node1).m_Property = trigger5;

			pFSM->NewProperty("1To2", FSMPropertyType::Trigger, trigger1);
			pFSM->NewProperty("1To3", FSMPropertyType::Trigger, trigger2);
			pFSM->NewProperty("2To3", FSMPropertyType::Trigger, trigger3);
			pFSM->NewProperty("3To4", FSMPropertyType::Trigger, trigger4);
			pFSM->NewProperty("4To1", FSMPropertyType::Trigger, trigger5);

			EditorAssetDatabase::CreateAsset(pFSM, finalPath.string());
			FileBrowserItem::GetSelectedFolder()->Refresh();
			FileBrowserItem::GetSelectedFolder()->SortChildren();
		});
	}

	void FSMEditorExtension::Initialize()
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		MainEditor& editor = pApp->GetMainEditor();
		pApp->GetMainEditor().RegisterMainWindow(&Editor);

		Importer::Register(&Importer);
		Tumbnail::AddGenerator<FSMTumbnailGenerator>();
		ObjectMenu::AddMenuItem("Create/Finite State Machine", OnCreateFSM, ObjectMenuType::T_ContentBrowser | ObjectMenuType::T_Resource | ObjectMenuType::T_Folder);
		MenuBar::AddMenuItem("Window/FSM Editor/Properties", [&editor]() { editor.GetWindow<FSMEditor, FSMPropertiesWindow>(); }, NULL, Shortcut_Window_FSMProperties);
		MenuBar::AddMenuItem("Window/FSM Editor/Nodes", [&editor]() { editor.GetWindow<FSMEditor, FSMNodeEditor>(); }, NULL, Shortcut_Window_FSMNodes);
		MenuBar::AddMenuItem("Window/FSM Editor/Inspector", [&editor]() { editor.GetWindow<FSMEditor, FSMNodeInspector>(); }, NULL, Shortcut_Window_FSMInspector);

		EditorPlayer::RegisterLoopHandler(this);

		FSMEditor* pMainWindow = editor.FindMainWindow<FSMEditor>();
		pMainWindow->GetWindow<FSMPropertiesWindow>();
		pMainWindow->GetWindow<FSMNodeEditor>();
		pMainWindow->GetWindow<FSMNodeInspector>();
	}

	const char* FSMEditorExtension::ModuleName()
	{
		return "Finite State Machines";
	}

	void FSMEditorExtension::HandleBeforeStart(Module* pModule)
	{
	}

	void FSMEditorExtension::HandleStart(Module* pModule)
	{
	}

	void FSMEditorExtension::HandleStop(Module* pModule)
	{
		FSMModule* pFSMModule = static_cast<FSMModule*>(pModule);
		pFSMModule->CleanupStates();
	}

	void FSMEditorExtension::HandleUpdate(Module* pModule)
	{
	}
}
